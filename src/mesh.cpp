#include <iostream>
#include <string>
#include "mesh.h"
#include "fileIO.h"


void Mesh::initialize(OBJData *objData)
{    
    assert((objData->numPositions % 3) == 0);
    for(size_t i = 0; i < objData->numPositions; i += 3)
        positions.push_back(Vec3(objData->positions[i], objData->positions[i+1], objData->positions[i + 2]));
    assert(objData->numPositions == positions.size() * 3);

    if(objData->numNormals > 0)
    {
        assert((objData->numNormals % 3) == 0);
        for(size_t i = 0; i < objData->numNormals; i += 3)
            normals.push_back(Vec3(objData->normals[i], objData->normals[i+1], objData->normals[i + 2]));
        assert(objData->numNormals == normals.size() * 3);
    }

    assert((objData->numTexcoords % 2) == 0);
    for(size_t i = 0; i < objData->numTexcoords; i += 2)
        texcoords.push_back(Vec2(objData->texcoords[i], objData->texcoords[i+1]));
    assert(objData->numTexcoords == texcoords.size() * 2);

    if(objData->numNormals > 0)
    {
        assert((objData->numFaces % 9) == 0);
        for(size_t i = 0; i < objData->numFaces; i += 9)
        {
            Face face;
            face[0] = FaceVertex(objData->faces[i], objData->faces[i+2], objData->faces[i+1]);
            face[1] = FaceVertex(objData->faces[i+3], objData->faces[i+5], objData->faces[i+4]);
            face[2] = FaceVertex(objData->faces[i+6], objData->faces[i+8], objData->faces[i+7]);
            faces.push_back(face);
        }
        assert(objData->numFaces == (faces.size() * 3 * 3));
    }else
    {
        assert((objData->numFaces % 6) == 0);
        for(size_t i = 0; i < objData->numFaces; i += 6)
        {
            Face face;
            face[0] = FaceVertex(objData->faces[i], -1, objData->faces[i+1]);
            face[1] = FaceVertex(objData->faces[i+2], -1, objData->faces[i+3]);
            face[2] = FaceVertex(objData->faces[i+4], -1, objData->faces[i+5]);
            faces.push_back(face);
        }
        assert(objData->numFaces == (faces.size() * 3 * 2));
    }

    if(objData->numGroups > 0)
    {
        for(size_t i = 0; i < objData->numGroups; i++)
        {
            std::string str(objData->mtlNames[i]);
            groups.push_back(std::make_pair(objData->groupIndices[i], str));
        }
    }
}


void Mesh::readFromObj(const char* fileName)
{
    OBJData objData;
    parseOBJFile(fileName, &objData);
    
    // if normArray is empty, normals vector will also be empty
    // the normal index in the faces entries will be a place holder
    /*
    initialize(posArray, normArray, texcoordArray, faceArray, groupFaceIndexArray, mtlNameArray, posArraySize, normArraySize,
               texcoordArraySize, faceIndex, groupCount);
    */
    initialize(&objData);

    // Clean up
    free(objData.positions);
    if(objData.numNormals > 0)
        free(objData.normals);
    if(objData.numTexcoords > 0)
        free(objData.texcoords);
    free(objData.faces);
    if(objData.numGroups > 0)
    {
        free(objData.groupIndices);
        for(size_t i = 0; i < objData.numGroups; i++)
            free(objData.mtlNames[i]);
        free(objData.mtlNames);
    }
}


void Mesh::computeVertexNormals()
{
    std::vector<Vec3> faceNorms;
    for(size_t i = 0; i < faces.size(); i++)
    {
        // compute face normal
        Face &face = faces[i];
        Vec3 pos0 = positions[face[0].posIndex];
        Vec3 pos1 = positions[face[1].posIndex];
        Vec3 pos2 = positions[face[2].posIndex];

        Vec3 normXAxis = pos1 - pos0;
        Vec3 tmpVec = pos2 - pos1;
        Vec3 normZAxis = cross(normXAxis, tmpVec);

        /*
        if(norm2(normZAxis) != 0.0f)
            normZAxis = normalize(normZAxis);
        */
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
        Face &face = faces[i];
        for(size_t j = 0; j < 3; j++)
        {
            normals[face[j].posIndex] += faceNorms[i];
            normTimes[face[j].posIndex] += 1.0f;
        }
    }

    for(size_t i = 0; i < normals.size(); i++)
    {
        // Divide each element in normals by the corresponding element in normTimes
        //normals[i] /= -normTimes[i];

        // Normalize each normal
        normals[i] = normalize(-normals[i]);
    }

    for(size_t i = 0; i < faces.size(); i++)
    {
        // Copy posIndex to normIndex
        Face &face = faces[i];
        for(size_t j = 0; j < 3; j++)
            face[j].normIndex = face[j].posIndex;
    }
    normalsComputed = true;
}

void Mesh::computeFaceNormals()
{
    std::vector<Vec3> faceNorms;
    for(size_t i = 0; i < faces.size(); i++)
    {
        // compute face normal
        Face face = faces[i];
        Vec3 pos0 = positions[face[0].posIndex];
        Vec3 pos1 = positions[face[1].posIndex];
        Vec3 pos2 = positions[face[2].posIndex];

        Vec3 normXAxis = pos1 - pos0;
        Vec3 tmpVec = pos2 - pos1;
        
        Vec3 normZAxis = cross(normXAxis, tmpVec);
        if(norm2(normZAxis) != 0.0f)
            normZAxis = normalize(normZAxis);
        faceNorms.push_back(normZAxis);
    }

    
    normals.clear();
    for(size_t i = 0; i < positions.size(); i++)
        normals.push_back(Vec3(0.0f, 0.0f, 0.0f));
    
    for(size_t i = 0; i < faces.size(); i++)
    {
        // for each face, cycle through all three face verts, and add the corresponding
        // face normal to normals at the same index as the posIndex
        // increment normTimes[posIndex]
        Face &face = faces[i];
        for(size_t j = 0; j < 3; j++)
        {
            normals[face[j].posIndex] = faceNorms[i];
        }
    }

     for(size_t i = 0; i < faces.size(); i++)
    {
        // Copy posIndex to normIndex
        Face &face = faces[i];
        for(size_t j = 0; j < 3; j++)
            face[j].normIndex = face[j].posIndex;
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
        //tangents[i] = tangents[i] - normals[i] * dot(normals[i], tangents[i]);        
        //binormals[i] = binormals[i] - normals[i] * dot(normals[i], binormals[i]);
        Vec3 tmpTan = tangents[i] - normals[i] * dot(normals[i], tangents[i]);
        Vec3 tmpBi = binormals[i] - normals[i] * dot(normals[i], binormals[i]);
        //tangents[i] = normalize(tangents[i]);
        //binormals[i] = normalize(binormals[i]);
        tangents[i] = normalize(tmpTan);
        binormals[i] = normalize(tmpBi);

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

size_t Mesh::geoListPNXTBD(Geometry ***GeoList, char ***mtlNames)
{
    return 0;
}

void Mesh::flipTexcoordY()
{
    for(size_t i = 0; i < texcoords.size(); i++)
        texcoords[i][1] = 1.0f - texcoords[i][1];
}


