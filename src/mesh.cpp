#include "mesh.h"

// TODO: temporary
// returns the index of the character immediately after the substring.
// returns -1 if no new substring
static int subStringAlpha(const char* fileContent, char buffer[], int fileSize, int index)
{
    int i, j;
    for(i = index; (fileContent[i] < 'A' || fileContent[i] > 'Z') && (fileContent[i] < 'a' || fileContent[i] > 'z'); i++)
    {
        if(i >= fileSize)
            return -1;
    }

    for(j = 0; (fileContent[j+i] >= 'A' && fileContent[j+i] <= 'Z') || (fileContent[j+i] >= 'a' && fileContent[j+i] <= 'z'); j++)
        buffer[j] = fileContent[j+i];

    buffer[j] = '\0';

    return j + i;
}

// TODO: temporary
// returns the index of the character immediately after the substring.
// returns -1 if no new substring
static int subStringNum(const char* fileContent, char buffer[], int fileSize, int index)
{
    int i, j;
    for(i = index; (fileContent[i] < '0' || fileContent[i] > '9') && fileContent[i] != '-'; i++)
    {
        if(i >= fileSize)
            return -1;
    }

    for(j = 0; ((fileContent[j+i] >= '0' && fileContent[j+i] <= '9') || fileContent[j+i] == '.') || fileContent[j+i] == '-'; j++)
        buffer[j] = fileContent[j+i];

    buffer[j] = '\0';
    return j + i;
}

void Mesh::initialize(const GLfloat *posArray, const GLfloat *normArray, const GLfloat *texcoordArray, const int *faceArray,
                     unsigned int posArraySize, unsigned int normArraySize, unsigned int texcoordArraySize,
                     unsigned int faceArraySize)
{
    assert((posArraySize % 3) == 0);
    for(unsigned int i = 0; i < posArraySize; i += 3)
        positions.push_back(Vec3(posArray[i], posArray[i+1], posArray[i + 2]));
    assert(posArraySize == positions.size() * 3);

    assert((normArraySize % 3) == 0);
    for(unsigned int i = 0; i < normArraySize; i += 3)
        normals.push_back(Vec3(normArray[i], normArray[i+1], normArray[i + 2]));
    assert(normArraySize == normals.size() * 3);

    assert((texcoordArraySize % 2) == 0);
    for(unsigned int i = 0; i < texcoordArraySize; i += 2)
        texcoords.push_back(Vec2(texcoordArray[i], texcoordArray[i+1]));
    assert(texcoordArraySize == texcoords.size() * 2);

    assert((faceArraySize % 9) == 0);
    for(unsigned int i = 0; i < faceArraySize; i += 9)
    {
        Face face;
        face[0] = FaceVertex(faceArray[i], faceArray[i+2], faceArray[i+1]);
        face[1] = FaceVertex(faceArray[i+3], faceArray[i+5], faceArray[i+4]);
        face[2] = FaceVertex(faceArray[i+6], faceArray[i+8], faceArray[i+7]);
        faces.push_back(face);
    }
    assert(faceArraySize == (faces.size() * 3 * 3));

    /*
      for(unsigned int i = 0; i < positions.size(); i++)
      {
      if((positions[i][0] != posArray[i*3] || positions[i][1] != posArray[i*3 + 1]) ||
      positions[i][2] != posArray[i*3 + 2])
      {
      fprintf(stderr, "Positions vector incorrect.\n");
      }
      }

      for(unsigned int i = 0; i < normals.size(); i++)
      {
      if((normals[i][0] != normArray[i*3] || normals[i][1] != normArray[i*3 + 1]) ||
      normals[i][2] != normArray[i*3 + 2])
      {
      fprintf(stderr, "Normals vector incorrect.\n");
      }
      }

      for(unsigned int i = 0; i < texcoords.size(); i++)
      {
      if((texcoords[i][0] != texcoordArray[i*2] || texcoords[i][1] != texcoordArray[i*2 + 1]))
      {
      fprintf(stderr, "Texcoords vector incorrect.\n");
      }
      }


      for(unsigned int i = 0; i < faces.size(); i++)
      {
      if((faces[i][0] != faceArray[i*3] || faces[i][1] != faceArray[i*3 + 1]) ||
      faces[i][2] != faceArray[i*3 + 2])
      {
      fprintf(stderr, "Faces vector incorrect.\n");
      }
      }
    */
}

// normalless version of initialize()
void Mesh::initialize(const GLfloat *posArray, const GLfloat *texcoordArray, const int *faceArray,
                     unsigned int posArraySize, unsigned int texcoordArraySize, unsigned int faceArraySize)
{
    assert((posArraySize % 3) == 0);
    for(unsigned int i = 0; i < posArraySize; i += 3)
        positions.push_back(Vec3(posArray[i], posArray[i+1], posArray[i + 2]));
    assert(posArraySize == positions.size() * 3);


    assert((texcoordArraySize % 2) == 0);
    for(unsigned int i = 0; i < texcoordArraySize; i += 2)
        texcoords.push_back(Vec2(texcoordArray[i], texcoordArray[i+1]));
    assert(texcoordArraySize == texcoords.size() * 2);

    assert((faceArraySize % 6) == 0);
    for(unsigned int i = 0; i < faceArraySize; i += 6)
    {
        Face face;
        face[0] = FaceVertex(faceArray[i], -1, faceArray[i+1]);
        face[1] = FaceVertex(faceArray[i+2], -1, faceArray[i+3]);
        face[2] = FaceVertex(faceArray[i+4], -1, faceArray[i+5]);
        faces.push_back(face);
    }
    assert(faceArraySize == (faces.size() * 3 * 2));
}

unsigned int Mesh::extractObjData(const char *fileContent, const int fileSize, GLfloat *posArray, GLfloat *normArray, GLfloat *texcoordArray, int *faceArray)
{
    // Iterate through fileContent and put data into the appropriate array
    char buffer[20];
    unsigned int posIndex = 0, normIndex = 0, texcoordIndex = 0, faceIndex = 0;
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
        } 
    }
    return faceIndex;
}

// returns the number of elements in faceArray
unsigned int Mesh::extractObjData(const char *fileContent, const int fileSize, GLfloat *posArray, GLfloat *texcoordArray, int *faceArray)
{
    // Iterate through fileContent and put data into the appropriate array
    char buffer[20];
    unsigned int posIndex = 0, texcoordIndex = 0, faceIndex = 0;
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
            // TODO: change for absent normals
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
        } 
    }
    return faceIndex;
}

// returns the number of elements in faceArray
void Mesh::readFromObj(const char* fileName)
{
    // Read the file into the string fileContent
    FILE *fp = fopen(fileName, "rb");
    if(fp == NULL)
    {
        fprintf(stderr, "Cannot open file.");
        exit(0);
    }
    
    fseek(fp, 0L, SEEK_END);
    int fileSize = ftell(fp);
    char *fileContent = (char*)malloc(sizeof(char) * fileSize + 1);
    rewind(fp);
    //printf("File size = %d\n", fileSize);

    int readResult = fread((void*)fileContent, 1, fileSize, fp);    
    if(readResult != fileSize)
    {
        fprintf(stderr, "Reading error.\n");
        exit(1);
    }
    fclose(fp);
    fileContent[readResult] = '\0';

    // Count the number of positions, normals, texcoords, and faces in the file
    // and allocate an array for each of them.
    char buffer[50];
    int posCount = 0, normCount = 0, texcoordCount = 0, faceCount = 0, index = 0;
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
            int slashCount = 0;
        }
    }

    /*
    printf("posCount = %d\n", posCount);
    printf("normCount = %d\n", normCount);
    printf("texcoordCount = %d\n", texcoordCount);
    printf("faceCount = %d\n", faceCount);
    */

    unsigned int posArraySize = posCount * 3;
    GLfloat *posArray = (GLfloat*)malloc(sizeof(GLfloat) * posArraySize);

    GLfloat *normArray;
    unsigned int normArraySize;
    if(normCount > 0)
    {
        normArraySize = normCount * 3;
        normArray = (GLfloat*)malloc(sizeof(GLfloat) * normArraySize);
    }

    unsigned int texcoordArraySize = texcoordCount * 2;
    GLfloat *texcoordArray = (GLfloat*)malloc(sizeof(GLfloat)*texcoordArraySize);

    // Not sure how many faces there will be after making sure thay are all triangles
    // so I quadrupled the size of the faceArray
    unsigned int faceArraySize = faceCount * 3 * 3 * 4;
    //faceArray = (int*)malloc(sizeof(int)*faceCount*3*3*2 *2);
    int *faceArray = (int*)malloc(sizeof(int)*faceArraySize);

    if(normCount > 0)
    {
        unsigned int faceIndex = extractObjData(fileContent, readResult, posArray, normArray, texcoordArray, faceArray);
        initialize(posArray, normArray, texcoordArray, faceArray, posArraySize, normArraySize, texcoordArraySize, faceIndex);
    }else
    {
        unsigned int faceIndex = extractObjData(fileContent, readResult, posArray, texcoordArray, faceArray);
        initialize(posArray, texcoordArray, faceArray, posArraySize, texcoordArraySize, faceIndex);
    }
    

    free(posArray);
    if(normCount > 0)
        free(normArray);
    free(texcoordArray);
    free(faceArray);
    free(fileContent);
}


void Mesh::computeVertexNormals()
{
    std::vector<Vec3> faceNorms;
    for(unsigned int i = 0; i < faces.size(); i++)
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
    for(unsigned int i = 0; i < positions.size(); i++)
        normals.push_back(Vec3(0.0f, 0.0f, 0.0f));
    
    for(unsigned int i = 0; i < faces.size(); i++)
    {
        // for each face, cycle through all three face verts, and add the corresponding
        // face normal to normals at the same index as the posIndex
        // increment normTimes[posIndex]
        for(int j = 0; j < 3; j++)
        {
            normals[faces[i][j].posIndex] += faceNorms[i];
            normTimes[faces[i][j].posIndex] += 1.0f;
        }
    }

    for(unsigned int i = 0; i < normals.size(); i++)
    {
        // Divide each element in normals by the corresponding element in normTimes
        normals[i] /= normTimes[i];
    }

    for(unsigned int i = 0; i < faces.size(); i++)
    {
        // Copy posIndex to normIndex
        for(int j = 0; j < 3; j++)
            faces[i][j].normIndex = faces[i][j].posIndex;
    }
    normalsComputed = true;
}

/*
  TODO: Only works if the normals are computed?
  Reference: 3D Math Primer p435
 */
void Mesh::computeVertexBasis()
{
    tangents.clear();
    binormals.clear();
    determinants.clear();
    for(unsigned int i = 0; i < normals.size(); i++)
    {
        tangents.push_back(Vec3(0.0f, 0.0f, 0.0f));
        binormals.push_back(Vec3(0.0f, 0.0f, 0.0f));
        determinants.push_back(0.0f);
    }
        
    for(unsigned int i = 0; i < faces.size(); i++)
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

        for(int j = 0; j < 3; j++)
        {
            tangents[faces[i][j].normIndex] += faceTangent;
            binormals[faces[i][j].normIndex] += faceBinormal;
        }
    }

    for(unsigned int i = 0; i < normals.size(); i++)
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

void Mesh::vertexAttribPNX(GLfloat *vertexArray, int *vertexIndex, int i, int j)
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

void Mesh::vertexAttribPNXTBD(GLfloat *vertexArray, int *vertexIndex, int i, int j)
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
    unsigned int vertexCount = int((float)faces.size() * 3 * 3 * (2.0f + 2.0f/3.0f));
    GLfloat *vertexArray = (GLfloat*)malloc(sizeof(GLfloat)*vertexCount);

    int vertexIndex = 0;
    for(unsigned int i = 0; i < faces.size(); i++)
    {
            
        vertexAttribPNX(vertexArray, &vertexIndex, i, 0);
        vertexAttribPNX(vertexArray, &vertexIndex, i, 1);
        vertexAttribPNX(vertexArray, &vertexIndex, i, 2);
    }
    assert((vertexIndex % 8) == 0);
    int numVertices = vertexIndex / 8;
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
    unsigned int vertexCount = int((float)faces.size() * 3 * (3 + 3 + 2 + 3 + 3 + 1));
    GLfloat *vertexArray = (GLfloat*)malloc(sizeof(GLfloat)*vertexCount);

    int vertexIndex = 0;
    for(unsigned int i = 0; i < faces.size(); i++)
    {
        vertexAttribPNXTBD(vertexArray, &vertexIndex, i, 0);
        vertexAttribPNXTBD(vertexArray, &vertexIndex, i, 1);
        vertexAttribPNXTBD(vertexArray, &vertexIndex, i, 2);            
    }
    assert((vertexIndex % 15) == 0);
    int numVertices = vertexIndex / 15;
    int vertSizePNXTBD = 15;

    Geometry *geometry = new Geometry(vertexArray, numVertices, vertSizePNXTBD);
    free(vertexArray);

    return geometry;
}
