#include <iostream>
#include <string>
#include "mesh.h"
#include "fileIO.h"


void Mesh::initialize(const GLfloat *posArray, const GLfloat *normArray, const GLfloat *texcoordArray, const size_t *faceArray,
                      const size_t *groupFaceIndexArray, char **mtlNameArray, const size_t posArraySize,
                      const size_t normArraySize, const size_t texcoordArraySize, const size_t faceArraySize,
                      const size_t numGroup)
{
    assert((posArraySize % 3) == 0);
    for(size_t i = 0; i < posArraySize; i += 3)
        positions.push_back(Vec3(posArray[i], posArray[i+1], posArray[i + 2]));
    assert(posArraySize == positions.size() * 3);

    if(normArraySize > 0)
    {
        assert((normArraySize % 3) == 0);
        for(size_t i = 0; i < normArraySize; i += 3)
            normals.push_back(Vec3(normArray[i], normArray[i+1], normArray[i + 2]));
        assert(normArraySize == normals.size() * 3);
    }

    assert((texcoordArraySize % 2) == 0);
    for(size_t i = 0; i < texcoordArraySize; i += 2)
        texcoords.push_back(Vec2(texcoordArray[i], texcoordArray[i+1]));
    assert(texcoordArraySize == texcoords.size() * 2);

    if(normArraySize > 0)
    {
        assert((faceArraySize % 9) == 0);
        for(size_t i = 0; i < faceArraySize; i += 9)
        {
            Face face;
            face[0] = FaceVertex(faceArray[i], faceArray[i+2], faceArray[i+1]);
            face[1] = FaceVertex(faceArray[i+3], faceArray[i+5], faceArray[i+4]);
            face[2] = FaceVertex(faceArray[i+6], faceArray[i+8], faceArray[i+7]);
            faces.push_back(face);
        }
        assert(faceArraySize == (faces.size() * 3 * 3));
    }else
    {
        assert((faceArraySize % 6) == 0);
        for(size_t i = 0; i < faceArraySize; i += 6)
        {
            Face face;
            face[0] = FaceVertex(faceArray[i], -1, faceArray[i+1]);
            face[1] = FaceVertex(faceArray[i+2], -1, faceArray[i+3]);
            face[2] = FaceVertex(faceArray[i+4], -1, faceArray[i+5]);
            faces.push_back(face);
        }
        assert(faceArraySize == (faces.size() * 3 * 2));
    }

    if(numGroup > 0)
    {
        for(size_t i = 0; i < numGroup; i++)
        {
            std::string str(mtlNameArray[i]);
            groups.push_back(std::make_pair(groupFaceIndexArray[i], str));
        }
    }
}


unsigned int Mesh::extractObjData(const char *fileContent, const size_t fileSize, GLfloat *posArray, GLfloat *normArray, GLfloat *texcoordArray, size_t *faceArray, size_t *groupFaceIndexArray, char **mtlNameArray)
{
    // Iterate through fileContent and put data into the appropriate array
    char buffer[20];
    size_t posIndex = 0, normIndex = 0, texcoordIndex = 0, faceIndex = 0, groupIndex = 0;
    int index = 0;
    while(index != -1)
    {
        index = subStringAlpha(fileContent, buffer, fileSize, index);
        if(strcmp(buffer, "v") == 0)
        {
            index = subStringNum(fileContent, buffer, fileSize, index);
            posArray[posIndex++] = (GLfloat)atof(buffer);
            index = subStringNum(fileContent, buffer, fileSize, index);
            posArray[posIndex++] = (GLfloat)atof(buffer);
            index = subStringNum(fileContent, buffer, fileSize, index);
            // Reversing Z
            posArray[posIndex++] = -(GLfloat)atof(buffer);
        }else if(strcmp(buffer, "vt") == 0)
        {
            index = subStringNum(fileContent, buffer, fileSize, index);
            texcoordArray[texcoordIndex++] = (GLfloat)atof(buffer);
            index = subStringNum(fileContent, buffer, fileSize, index);
            // Reversing V
            texcoordArray[texcoordIndex++] = 1.0f - (GLfloat)atof(buffer);
        }else if(strcmp(buffer, "vn") == 0)
        {
            index = subStringNum(fileContent, buffer, fileSize, index);
            normArray[normIndex++] = (GLfloat)atof(buffer);
            index = subStringNum(fileContent, buffer, fileSize, index);
            normArray[normIndex++] = (GLfloat)atof(buffer);
            index = subStringNum(fileContent, buffer, fileSize, index);
            normArray[normIndex++] = -(GLfloat)atof(buffer);
        }else if(strcmp(buffer, "f") == 0)
        {
            // If a face is a quad, it's split into two triangles.
            // If it's a pentagon, it's split into three triangles.
            int slashCount = 0;
 
            // Count the number of slashes in the line to determine what polygon the face is.
            for(int i = index; fileContent[i] != '\n'; i++)
            {
                if(fileContent[i] == '/')
                    slashCount++;
            }

            int numFaceVerts = slashCount / 2;
            int tmpIndex;
            for(int i = 0; i < (numFaceVerts - 2); i++)
            {
                tmpIndex = index;
                for(int j = 0; j < (9 + i*3); j++)
                {
                    tmpIndex = subStringNum(fileContent, buffer, fileSize, tmpIndex);
                    if(j < 3 || j > (2 + i*3))
                        faceArray[faceIndex++] = atoi(buffer) - 1;
                }
            }
        }else if(strcmp(buffer, "g") == 0)
        {

            groupFaceIndexArray[groupIndex] = faceIndex / (3 * 3); // 3 indices per vertex and 3 vertices per face

            while(fileContent[index++] != '\n'); // Skip over group name
            // Assuming "usemtl blah_material" is the next line
            index = subStringAlpha(fileContent, buffer, fileSize, index); // Skip over "usemtl"

            index = getMaterialName(fileContent, buffer, fileSize, index);

            strcpy(mtlNameArray[groupIndex++], buffer);
        }
    }
    return faceIndex;
}

// TODO: get rid of this one
// returns the number of elements in faceArray
unsigned int Mesh::extractObjData(const char *fileContent, const size_t fileSize, GLfloat *posArray, GLfloat *texcoordArray, size_t *faceArray, size_t *groupFaceIndexArray, char **mtlNameArray)
{
    // Iterate through fileContent and put data into the appropriate array
    char buffer[20];
    size_t posIndex = 0, texcoordIndex = 0, faceIndex = 0, groupIndex = 0;
    int index = 0;
    while(index != -1)
    {
        index = subStringAlpha(fileContent, buffer, fileSize, index);
        if(strcmp(buffer, "v") == 0)
        {
            index = subStringNum(fileContent, buffer, fileSize, index);
            posArray[posIndex++] = (GLfloat)atof(buffer);
            index = subStringNum(fileContent, buffer, fileSize, index);
            posArray[posIndex++] = (GLfloat)atof(buffer);
            index = subStringNum(fileContent, buffer, fileSize, index);
            posArray[posIndex++] = (GLfloat)atof(buffer);
        }else if(strcmp(buffer, "vt") == 0)
        {
            index = subStringNum(fileContent, buffer, fileSize, index);
            texcoordArray[texcoordIndex++] = (GLfloat)atof(buffer);
            index = subStringNum(fileContent, buffer, fileSize, index);
            texcoordArray[texcoordIndex++] = (GLfloat)atof(buffer);
        }else if(strcmp(buffer, "f") == 0)
        {
            // If a face is a quad, it's split into two triangles.
            // If it's a pentagon, it's split into three triangles.
            int slashCount = 0;
 
            // Count the number of slashes in the line to determine what polygon the face is.
            for(int i = index; fileContent[i] != '\n'; i++)
            {
                if(fileContent[i] == '/')
                    slashCount++;
            }

            //int numFaceVerts = slashCount / 2; 
            int numFaceVerts = slashCount;
            int tmpIndex;
            for(int i = 0; i < (numFaceVerts - 2); i++)
            {
                tmpIndex = index;
                //for(int j = 0; j < (9 + i*3); j++)
                for(int j = 0; j < (6 + i*2); j++)
                {
                    tmpIndex = subStringNum(fileContent, buffer, fileSize, tmpIndex);
                    //if(j < 3 || j > (2 + i*3))
                    if(j < 2 || j > (1 + i*2))
                        faceArray[faceIndex++] = atoi(buffer) - 1;
                }
            }
        }else if(strcmp(buffer, "g") == 0)
        {
            groupFaceIndexArray[groupIndex] = faceIndex / (2 * 3); // 2 indices per vertex and 3 vertices per face
            while(fileContent[index++] != '\n'); // Skip over group name
            // Assuming "usemtl blah_material" is the next line
            index = subStringAlpha(fileContent, buffer, fileSize, index); // Skip over "usemtl"
            index = getMaterialName(fileContent, buffer, fileSize, index); 
            strcpy(mtlNameArray[groupIndex++], buffer);
        } 
    }
    return faceIndex;
}

// returns the number of elements in faceArray
void Mesh::readFromObj(const char* fileName)
{
    // Read the file into the string fileContent
    char *fileContent;
    size_t readResult = readFileIntoString(fileName, &fileContent);
    

    // Count the number of positions, normals, texcoords, faces, and groups in the file
    char buffer[50];
    size_t posCount = 0, normCount = 0, texcoordCount = 0, faceCount = 0;
    int index = 0;
    size_t groupCount = 0;
    while(index != -1)
    {
        index = subStringAlpha(fileContent, buffer, readResult, index);
        if(strcmp(buffer, "v") == 0)
            posCount++;
        else if(strcmp(buffer, "vt") == 0)
            texcoordCount++;
        else if(strcmp(buffer, "vn") == 0)            
            normCount++;
        else if(strcmp(buffer, "f") == 0 && fileContent[index - 2] == '\n')
        {
            faceCount++;
        }else if(strcmp(buffer, "g") == 0)
        {
            groupCount++;
        }
    }


    // Allocate arrays for positions, normals, texcoords, faces, and groups
    size_t posArraySize = 0, normArraySize = 0, texcoordArraySize = 0, faceArraySize = 0;
    GLfloat *posArray, *normArray, *texcoordArray;
    size_t *faceArray, *groupFaceIndexArray;
    char **mtlNameArray;

    // Initialize the variables above
    {
        posArraySize = posCount * 3;
        posArray = (GLfloat*)malloc(sizeof(GLfloat) * posArraySize);

        if(normCount > 0)
        {
            normArraySize = normCount * 3;
            normArray = (GLfloat*)malloc(sizeof(GLfloat) * normArraySize);
        }

        texcoordArraySize = texcoordCount * 2;
        texcoordArray = (GLfloat*)malloc(sizeof(GLfloat)*texcoordArraySize);

        // Not sure how many faces there will be after making sure thay are all triangles
        // so I quadrupled the size of the faceArray
        faceArraySize = faceCount * 3 * 3 * 4;
        //faceArray = (int*)malloc(sizeof(int)*faceCount*3*3*2 *2);
        faceArray = (size_t*)malloc(sizeof(size_t)*faceArraySize);

        if(groupCount > 0)
        {
            groupFaceIndexArray = (size_t*)malloc(sizeof(size_t)*groupCount);
            mtlNameArray = (char**)malloc(sizeof(char*)*groupCount);
            for(size_t i = 0; i < groupCount; i++)
                mtlNameArray[i] = (char*)malloc(sizeof(char*)*20);
        }
    }


    size_t faceIndex;
    if(normCount > 0)
        faceIndex = extractObjData(fileContent, readResult, posArray, normArray, texcoordArray, faceArray, groupFaceIndexArray,
                                   mtlNameArray);
    else
        faceIndex = extractObjData(fileContent, readResult, posArray, texcoordArray, faceArray, groupFaceIndexArray, mtlNameArray);

    // if normArray is empty, normals vector will also be empty
    // the normal index in the faces entries will be a place holder
    initialize(posArray, normArray, texcoordArray, faceArray, groupFaceIndexArray, mtlNameArray, posArraySize, normArraySize,
               texcoordArraySize, faceIndex, groupCount);

    // Clean up
    free(posArray);
    if(normCount > 0)
        free(normArray);
    free(texcoordArray);
    free(faceArray);
    if(groupCount > 0)
    {
        free(groupFaceIndexArray);
        for(size_t i = 0; i < groupCount; i++)
            free(mtlNameArray[i]);
        free(mtlNameArray);
    }
    free(fileContent);
}


void Mesh::computeVertexNormals()
{
    std::vector<Vec3> faceNorms;
    for(size_t i = 0; i < faces.size(); i++)
    {
        // compute face normal
        Vec3 pos0 = positions[faces[i][0].posIndex];
        Vec3 pos1 = positions[faces[i][1].posIndex];
        Vec3 pos2 = positions[faces[i][2].posIndex];
        Face f = faces[i];
        Vec3 normXAxis = positions[faces[i][1].posIndex] - positions[faces[i][0].posIndex];
        Vec3 tmpVec = positions[faces[i][2].posIndex] - positions[faces[i][0].posIndex];
        Vec3 normZAxis = cross(normXAxis, tmpVec);
        if(norm2(normZAxis) != 0.0f)
            normZAxis = normalize(normZAxis);
        faceNorms.push_back(normZAxis);
    }

    
    std::vector<float> normTimes(positions.size(), 0.0);    
    normals.clear();
    for(size_t i = 0; i < positions.size(); i++)
        normals.push_back(Vec3(0.0f, 0.0f, 0.0f));
    
    for(size_t i = 0; i < faces.size(); i++)
    {
        // for each face, cycle through all three face verts, and add the corresponding
        // face normal to normals at the same index as the posIndex
        // increment normTimes[posIndex]
        for(size_t j = 0; j < 3; j++)
        {
            normals[faces[i][j].posIndex] += faceNorms[i];
            normTimes[faces[i][j].posIndex] += 1.0f;
        }
    }

    for(size_t i = 0; i < normals.size(); i++)
    {
        // Divide each element in normals by the corresponding element in normTimes
        normals[i] /= normTimes[i];
    }

    for(size_t i = 0; i < faces.size(); i++)
    {
        // Copy posIndex to normIndex
        for(size_t j = 0; j < 3; j++)
            faces[i][j].normIndex = faces[i][j].posIndex;
    }
    normalsComputed = true;
}

/*
  Reference: 3D Math Primer p435
 */
void Mesh::computeVertexBasis()
{
    tangents.clear();
    binormals.clear();
    determinants.clear();
    for(size_t i = 0; i < normals.size(); i++)
    {
        tangents.push_back(Vec3(0.0f, 0.0f, 0.0f));
        binormals.push_back(Vec3(0.0f, 0.0f, 0.0f));
        determinants.push_back(0.0f);
    }
        
    for(size_t i = 0; i < faces.size(); i++)
    {
        // for each face, calculate its tangent and binormal
        // then add them to the corresponding entries in the tangents vector and binormals vector
        // for each adjacent vertex.
        Vec3 q1 = positions[faces[i][1].posIndex] - positions[faces[i][0].posIndex];
        Vec3 q2 = positions[faces[i][2].posIndex] - positions[faces[i][0].posIndex];        

        float s1 = (texcoords[faces[i][1].texcoordIndex])[0] - (texcoords[faces[i][0].texcoordIndex])[0];
        float s2 = (texcoords[faces[i][2].texcoordIndex])[0] - (texcoords[faces[i][0].texcoordIndex])[0];
        float t1 = (texcoords[faces[i][1].texcoordIndex])[1] - (texcoords[faces[i][0].texcoordIndex])[1];
        float t2 = (texcoords[faces[i][2].texcoordIndex])[1] - (texcoords[faces[i][0].texcoordIndex])[1];

        Vec3 faceTangent = q1*t2 - q2*t1;
        Vec3 faceBinormal = q2*s1 - q1*s2;

        for(size_t j = 0; j < 3; j++)
        {
            tangents[faces[i][j].normIndex] += faceTangent;
            binormals[faces[i][j].normIndex] += faceBinormal;
        }
    }

    for(size_t i = 0; i < normals.size(); i++)
    {
        // Make sure that the tangent and binormal vectors are perpendicular to the vertex normal
        // then normalize them
        tangents[i] = tangents[i] - normals[i] * dot(normals[i], tangents[i]);        
        binormals[i] = binormals[i] - normals[i] * dot(normals[i], binormals[i]);
        tangents[i] = normalize(tangents[i]);
        binormals[i] = normalize(binormals[i]);

        // Check if we're mirrored
        if(dot(cross(normals[i], tangents[i]), binormals[i]) < 0.0f)
            determinants[i] = -1.0f;   // mirrored
        else
            determinants[i] = 1.0f;    // not mirrored
    }
}

void Mesh::vertexAttribPNX(GLfloat *vertexArray, size_t *vertexIndex, const size_t i, const size_t j)
{
    // NOTE: what's with all the reversal?
    // the z component of position and normal are both reversed
    // normal used to be untouched, and the object was lit on the wrong side

    // Position
    vertexArray[(*vertexIndex)++] = positions[faces[i][j].posIndex][0];
    vertexArray[(*vertexIndex)++] = positions[faces[i][j].posIndex][1];
    vertexArray[(*vertexIndex)++] = positions[faces[i][j].posIndex][2];
    // Normal
    vertexArray[(*vertexIndex)++] = normals[faces[i][j].normIndex][0];
    vertexArray[(*vertexIndex)++] = normals[faces[i][j].normIndex][1];
    vertexArray[(*vertexIndex)++] = normals[faces[i][j].normIndex][2];
    // Texcoord
    // also reversed the v component of texcoord
    vertexArray[(*vertexIndex)++] = texcoords[faces[i][j].texcoordIndex][0];
    vertexArray[(*vertexIndex)++] = texcoords[faces[i][j].texcoordIndex][1];
}

void Mesh::vertexAttribPNXTBD(GLfloat *vertexArray, size_t *vertexIndex, const size_t i, const size_t j)
{
    vertexAttribPNX(vertexArray, vertexIndex, i, j);
    // Tangent
    vertexArray[(*vertexIndex)++] = tangents[faces[i][j].normIndex][0];
    vertexArray[(*vertexIndex)++] = tangents[faces[i][j].normIndex][1];
    vertexArray[(*vertexIndex)++] = tangents[faces[i][j].normIndex][2];
    // Binormal
    vertexArray[(*vertexIndex)++] = binormals[faces[i][j].normIndex][0];
    vertexArray[(*vertexIndex)++] = binormals[faces[i][j].normIndex][1];
    vertexArray[(*vertexIndex)++] = binormals[faces[i][j].normIndex][2];
    // Determinant
    vertexArray[(*vertexIndex)++] = determinants[faces[i][j].normIndex];
}

/*
  Returns a Geoemetry* with its VBO in the format of:
  position, normal, texcoord
 */
Geometry* Mesh::produceGeometryPNX()
{
    if(((positions.size() == 0 || normals.size() == 0) || texcoords.size() == 0) || faces.size() == 0)
    {
        fprintf(stderr, "Mesh doesn't contain necessary data to produce a Geometry object.\n");
        return NULL;
    }

    /*
      NOTE: vertexCount used to be 1/3 of the correct number, so vertexArray was also 1/3 of the correct size.
      However, in the for loop below, instead of i++, I had it as i+=3, so it all somehow fit?
      What it caused, in addition to not having the correct geometry, is heap corruption (not sure how).
      In the function that called this one, freeing unrelated memory would cause a heap corruption error.
    */
    size_t vertexCount = size_t((float)faces.size() * 3 * 3 * (2.0f + 2.0f/3.0f));
    GLfloat *vertexArray = (GLfloat*)malloc(sizeof(GLfloat)*vertexCount);

    size_t vertexIndex = 0;
    for(size_t i = 0; i < faces.size(); i++)
    {
            
        vertexAttribPNX(vertexArray, &vertexIndex, i, 0);
        vertexAttribPNX(vertexArray, &vertexIndex, i, 1);
        vertexAttribPNX(vertexArray, &vertexIndex, i, 2);
    }
    assert((vertexIndex % 8) == 0);
    size_t numVertices = vertexIndex / 8;
    int vertSizePNX = 8;

    Geometry *geometry = new Geometry(vertexArray, numVertices, vertSizePNX);
    free(vertexArray);

    return geometry;
}

/*
  Returns a Geoemetry* with its VBO in the format of:
  position, normal, texcoord, tangent, binormal, determinant
 */
Geometry* Mesh::produceGeometryPNXTBD()
{
    if(((positions.size() == 0 || normals.size() == 0) || texcoords.size() == 0) || faces.size() == 0)
    {
        fprintf(stderr, "Mesh doesn't contain necessary data to produce a Geometry object.\n");
        return NULL;
    }

    //unsigned int vertexCount = int((float)faces.size() * 3 * 3 * (2.0f + 2.0f/3.0f));
    // (3 + 3 + 2 + 3 + 3 + 1) = size per vertex
    // position + normal + texcoord + tangent + binormal + determinant
    size_t vertexCount = size_t((float)faces.size() * 3 * (3 + 3 + 2 + 3 + 3 + 1));
    GLfloat *vertexArray = (GLfloat*)malloc(sizeof(GLfloat)*vertexCount);

    size_t vertexIndex = 0;
    for(size_t i = 0; i < faces.size(); i++)
    {
        vertexAttribPNXTBD(vertexArray, &vertexIndex, i, 0);
        vertexAttribPNXTBD(vertexArray, &vertexIndex, i, 1);
        vertexAttribPNXTBD(vertexArray, &vertexIndex, i, 2);            
    }
    assert((vertexIndex % 15) == 0);
    size_t numVertices = vertexIndex / 15;
    int vertSizePNXTBD = 15;

    Geometry *geometry = new Geometry(vertexArray, numVertices, vertSizePNXTBD);
    free(vertexArray);

    return geometry;
}

Geometry* Mesh::geometryFromGroupPNX(size_t groupNum)
{
    if(groupNum >= groups.size())
    {
        fprintf(stderr, "Invalid group number.\n");
        return NULL;
    }
    size_t startFaceIndex = groups[groupNum].first;
    size_t endFaceIndex;

    if(groupNum == groups.size() - 1)
        endFaceIndex = faces.size();
    else
        endFaceIndex = groups[groupNum + 1].first;

    if(startFaceIndex == endFaceIndex)
    {
        fprintf(stderr, "Invalid face index interval\n");
        return NULL;
    }

    size_t vertexCount = size_t((float)(endFaceIndex - startFaceIndex + 1) * 3 * 3 * (2.0f + 2.0f/3.0f));
    GLfloat *vertexArray = (GLfloat*)malloc(sizeof(GLfloat)*vertexCount);

    size_t vertexIndex = 0;
    for(size_t i = startFaceIndex; i < endFaceIndex; i++)
    {            
        vertexAttribPNX(vertexArray, &vertexIndex, i, 0);
        vertexAttribPNX(vertexArray, &vertexIndex, i, 1);
        vertexAttribPNX(vertexArray, &vertexIndex, i, 2);
    }
    assert((vertexIndex % 8) == 0);
    size_t numVertices = vertexIndex / 8;
    int vertSizePNX = 8;

    Geometry *geometry = new Geometry(vertexArray, numVertices, vertSizePNX);
    free(vertexArray);

    return geometry;    
}

size_t Mesh::geoListPNX(Geometry ***GeoList, char ***mtlNames)    
{
    if(groups.size() == 0)
    {
        fprintf(stderr, "No groups.\n");
        return 0;
    }

    *GeoList = (Geometry**)malloc(sizeof(Geometry*)*groups.size());
    *mtlNames = (char**)malloc(sizeof(char*)*groups.size());

    for(size_t i = 0; i < groups.size(); i++)
    {
        (*GeoList)[i] = geometryFromGroupPNX(i);
        (*mtlNames)[i] = (char*)malloc(sizeof(char)*groups[i].second.length());
        strcpy((*mtlNames)[i], groups[i].second.c_str());
    }
    return groups.size();
}

void Mesh::flipTexcoordY()
{
    for(size_t i = 0; i < texcoords.size(); i++)
        texcoords[i][1] = 1.0f - texcoords[i][1];
}

