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

    assert((faceArraySize % 9) == 0);
    for(unsigned int i = 0; i < faceArraySize; i += 9)
    {
        Face face;
        face[0] = FaceVertex(faceArray[i], -1, faceArray[i+1]);
        face[1] = FaceVertex(faceArray[i+3], -1, faceArray[i+4]);
        face[2] = FaceVertex(faceArray[i+6], -1, faceArray[i+7]);
        faces.push_back(face);
    }
    assert(faceArraySize == (faces.size() * 3 * 3));
}

int Mesh::extractObjData(const char *fileContent, const int fileSize, GLfloat *posArray, GLfloat *normArray, GLfloat *texcoordArray, int *faceArray)
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
            posArray[posIndex++] = (GLfloat)atof(buffer);
        }else if(strcmp(buffer, "vt") == 0)
        {
            index = subStringNum(fileContent, buffer, fileSize, index);
            texcoordArray[texcoordIndex++] = (GLfloat)atof(buffer);
            index = subStringNum(fileContent, buffer, fileSize, index);
            texcoordArray[texcoordIndex++] = (GLfloat)atof(buffer);
        }else if(strcmp(buffer, "vn") == 0)
        {
            index = subStringNum(fileContent, buffer, fileSize, index);
            normArray[normIndex++] = (GLfloat)atof(buffer);
            index = subStringNum(fileContent, buffer, fileSize, index);
            normArray[normIndex++] = (GLfloat)atof(buffer);
            index = subStringNum(fileContent, buffer, fileSize, index);
            normArray[normIndex++] = (GLfloat)atof(buffer);
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
int Mesh::extractObjData(const char *fileContent, const int fileSize, GLfloat *posArray, GLfloat *texcoordArray, int *faceArray)
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
void Mesh::readFromObj(const char* fileName, int *numVertices)
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
    printf("File size = %d\n", fileSize);

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

    printf("posCount = %d\n", posCount);
    printf("normCount = %d\n", normCount);
    printf("texcoordCount = %d\n", texcoordCount);
    printf("faceCount = %d\n", faceCount);

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
        int faceIndex = extractObjData(fileContent, readResult, posArray, normArray, texcoordArray, faceArray);
        initialize(posArray, normArray, texcoordArray, faceArray, posArraySize, normArraySize, texcoordArraySize, faceIndex);
    }else
    {
        int faceIndex = extractObjData(fileContent, readResult, posArray, texcoordArray, faceArray);
        initialize(posArray, texcoordArray, faceArray, posArraySize, texcoordArraySize, faceIndex);
    }
    

    free(posArray);
    if(normCount > 0)
        free(normArray);
    free(texcoordArray);
    free(faceArray);
    free(fileContent);
}

void Mesh::computeNormals()
{
    std::vector<Vec3> faceNorms;
    unsigned int faceSize = faces.size();
    for(unsigned int i = 0; i < faceSize; i++)
    {
        // TODO: compute face normal
    }

    std::vector<float> normTimes(positions.size(), 0.0);
    unsigned int faceNormSize = faceSize;
    for(unsigned int i = 0; i < faceNormSize; i++)
    {
        // TODO: for each face, cycle through all three face verts, and add the corresponding
        // face normal to normals at the same index as the posIndex
        // increment normTimes[posIndex]
    }

    unsigned normSize = normals.size();
    for(unsigned int i = 0; i < normSize; i++)
    {
        // TODO: divide each element in normals by the corresponding element in normTimes
    }

    for(unsigned int i = 0; i < faceSize; i++)
    {
        // TODO: copy posIndex to normIndex
    }
}
