#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

// Not sure why 
#define MAX_VERTICES 8000
#define MAX_POLYGONS 8000

struct Geometry {
    GLuint vao, vbo, ebo;
    int vboLen = 0, eboLen = 0;
    GLuint shaderProgram;

    Geometry(GLfloat vtx[], GLuint edx[], int vboLen, int eboLen) {
        this->vboLen = vboLen;
        this->eboLen = eboLen;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);
        // Now create the VBO and IBO
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vboLen * 8, vtx, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * eboLen, edx, GL_STATIC_DRAW);
        
        glBindVertexArray(0);
    }

    Geometry(GLfloat vtx[], int vboLen) {
        this->vboLen = vboLen;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vboLen * 8, vtx, GL_STATIC_DRAW);

        glBindVertexArray(0);
    }
};

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


//void readFromCollada(const char* fileName, Geometry *geometry)
GLfloat* readFromCollada(const char* fileName, int *numVertices)
{


    char buffer[50], *fileContent;
    int i, posCount, normCount, indexCount, fileSize, readResult, index;
    float *posArray, *normArray, *vertexArray;
    int *indexArray;
    
    // Determine the size of the file
    FILE *fp = fopen(fileName, "rb");
    if(fp == NULL)
    {
        fprintf(stderr, "Cannot open file.");
        exit(0);
    }
    
    fseek(fp, 0L, SEEK_END);
    fileSize = ftell(fp);
    fileContent = (char*)malloc(sizeof(char) * fileSize + 1);
    rewind(fp);
    printf("File size = %d\n", fileSize);

    readResult = fread((void*)fileContent, 1, fileSize, fp);
    if(readResult != fileSize)
    {
        fprintf(stderr, "Reading error.\n");
        exit(1);
    }
    fclose(fp);
    
    // Read in the positions
    index = 0;
    while(strcmp(buffer, "count") != 0)
        index = subStringAlpha(fileContent, buffer, readResult, index);

    index = subStringNum(fileContent, buffer, readResult, index);
    posCount = atoi(buffer);
    printf("posCount = %d\n", posCount);
    posArray = (float*)malloc(sizeof(float) * posCount);
    for(i = 0; i < posCount; i++)
    {
        index = subStringNum(fileContent, buffer, readResult, index);
        posArray[i] = (float)atof(buffer);
    }

    // Read in the normals
    while(strcmp(buffer, "normals") != 0)
        index = subStringAlpha(fileContent, buffer, readResult, index);

    while(strcmp(buffer, "count") != 0)
        index = subStringAlpha(fileContent, buffer, readResult, index);

    index = subStringNum(fileContent, buffer, readResult, index);
    normCount = atoi(buffer);
    printf("normCount = %d\n", normCount);
    normArray = (float*)malloc(sizeof(float) * normCount);
    for(i = 0; i < normCount; i++)
    {
        index = subStringNum(fileContent, buffer, readResult, index);
        normArray[i] = (float)atof(buffer);
    }

    // Read in the indices
    while(strcmp(buffer, "vcount") != 0)
        index = subStringAlpha(fileContent, buffer, readResult, index);

    index = subStringAlpha(fileContent, buffer, readResult, index);

    indexCount = normCount * 2;
    indexArray = (int*)malloc(sizeof(int) * indexCount);
    printf("new indexCount = %d\n", indexCount);
    for(i = 0; i < indexCount; i++)
    {
        index = subStringNum(fileContent, buffer, readResult, index);
        indexArray[i] = atoi(buffer);
    }
    // Stuff positions and normals into the final vertex attribute array
    // multiplied by 4 to account for texcoord
    vertexArray = (float*)malloc(sizeof(float) * indexCount * 4);
    
    for(i = 0; i < indexCount/2; i++)
    {
        // position
        // y and z switched
        vertexArray[i*8] = posArray[indexArray[i*2]*3];
        vertexArray[i*8 + 1] = posArray[indexArray[i*2]*3 + 2];
        vertexArray[i*8 + 2] = posArray[indexArray[i*2]*3 + 1];
        // normal
        // y and z switched
        vertexArray[i*8 + 3] = normArray[indexArray[i*2 + 1]*3];
        vertexArray[i*8 + 4] = normArray[indexArray[i*2 + 1]*3 + 2];
        vertexArray[i*8 + 5] = normArray[indexArray[i*2 + 1]*3 + 1];
        // texcoord
        vertexArray[i*8 + 6] = 0.0f;
        vertexArray[i*8 + 7] = 0.0f;
    }

    free(posArray);
    free(normArray);
    free(indexArray);
    free(fileContent);

    *numVertices = indexCount/2;
    return vertexArray;
}

GLfloat* readFromObj(const char* fileName, int *numVertices)
{
    int posCount, normCount, indexCount, texcoordCount, faceCount, vertexCount, fileSize, readResult, index;
    char *fileContent, buffer[50];;
    GLfloat *posArray, *normArray, *texcoordArray; 
    int *faceArray;
    GLfloat *vertexArray;
    
    // Determine the size of the file
    FILE *fp = fopen(fileName, "rb");
    if(fp == NULL)
    {
        fprintf(stderr, "Cannot open file.");
        exit(0);
    }
    
    fseek(fp, 0L, SEEK_END);
    fileSize = ftell(fp);
    fileContent = (char*)malloc(sizeof(char) * fileSize + 1);
    rewind(fp);
    printf("File size = %d\n", fileSize);

    readResult = fread((void*)fileContent, 1, fileSize, fp);
    
    if(readResult != fileSize)
    {
        fprintf(stderr, "Reading error.\n");
        exit(1);
    }
    fclose(fp);

    fileContent[readResult] = '\0';

    posCount = normCount = texcoordCount  = faceCount = 0;
    index = 0;
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
    
    posArray = (GLfloat*)malloc(sizeof(GLfloat)*posCount*3);
    normArray = (GLfloat*)malloc(sizeof(GLfloat)*normCount*3);
    texcoordArray = (GLfloat*)malloc(sizeof(GLfloat)*texcoordCount*2);
    faceArray = (int*)malloc(sizeof(int)*faceCount*3*3*2);

    int posIndex = 0, normIndex = 0, texcoordIndex = 0, faceIndex = 0;

    index = 0;

    while(index != -1)
    {
        index = subStringAlpha(fileContent, buffer, readResult, index);
        if(strcmp(buffer, "v") == 0)
        {
            index = subStringNum(fileContent, buffer, readResult, index);
            posArray[posIndex++] = (GLfloat)atof(buffer);
            index = subStringNum(fileContent, buffer, readResult, index);
            posArray[posIndex++] = (GLfloat)atof(buffer);
            index = subStringNum(fileContent, buffer, readResult, index);
            posArray[posIndex++] = (GLfloat)atof(buffer);
        }else if(strcmp(buffer, "vt") == 0)
        {
            index = subStringNum(fileContent, buffer, readResult, index);
            texcoordArray[texcoordIndex++] = (GLfloat)atof(buffer);
            index = subStringNum(fileContent, buffer, readResult, index);
            texcoordArray[texcoordIndex++] = (GLfloat)atof(buffer);
        }else if(strcmp(buffer, "vn") == 0)
        {
            index = subStringNum(fileContent, buffer, readResult, index);
            normArray[normIndex++] = (GLfloat)atof(buffer);
            index = subStringNum(fileContent, buffer, readResult, index);
            normArray[normIndex++] = (GLfloat)atof(buffer);
            index = subStringNum(fileContent, buffer, readResult, index);
            normArray[normIndex++] = (GLfloat)atof(buffer);
        }else if(strcmp(buffer, "f") == 0)
        {
            int slashCount = 0;

            for(int i = index; fileContent[i] != '\n'; i++)
            {
                if(fileContent[i] == '/')
                    slashCount++;
            }
            if(maxSlash < slashCount)
                maxSlash = slashCount;

            int tmpIndex = index;
            for(int i = 0; i < 9; i++)
            {
                tmpIndex = subStringNum(fileContent, buffer, readResult, tmpIndex);
                faceArray[faceIndex++] = atoi(buffer);
            }

            if(slashCount >= 8)
            {
                tmpIndex = index;
                for(int i = 0; i < 12; i++)
                {
                    tmpIndex = subStringNum(fileContent, buffer, readResult, tmpIndex);
                    if(i < 3 || i > 5)
                        faceArray[faceIndex++] = atoi(buffer);
                }
            }

            if(slashCount >= 10)
            {
                tmpIndex = index;
                for(int i = 0; i < 15; i++)
                {
                    tmpIndex = subStringNum(fileContent, buffer, readResult, tmpIndex);
                    if(i < 3 || i > 8)
                        faceArray[faceIndex++] = atoi(buffer);
                }
            }
        } 
    }

    vertexCount  = int((float)faceIndex*(2.0f + 2.0f/3.0f));
    vertexArray = (GLfloat*)malloc(sizeof(GLfloat)*vertexCount);

    int vertexIndex = 0;
    for(int i = 0; i < faceIndex; i+=3)
    {
        vertexArray[vertexIndex++] = posArray[(faceArray[i]-1)*3];
        vertexArray[vertexIndex++] = posArray[(faceArray[i]-1)*3 + 1];
        vertexArray[vertexIndex++] = posArray[(faceArray[i]-1)*3 + 2];

        vertexArray[vertexIndex++] = normArray[(faceArray[i + 2]-1)*3];
        vertexArray[vertexIndex++] = normArray[(faceArray[i + 2]-1)*3 + 1];
        vertexArray[vertexIndex++] = normArray[(faceArray[i + 2]-1)*3 + 2];

        vertexArray[vertexIndex++] = texcoordArray[(faceArray[i + 1]-1)*2];
        vertexArray[vertexIndex++] = texcoordArray[(faceArray[i + 1]-1)*2 + 1];
    }

    *numVertices = vertexIndex / 8;
    
    free(posArray);
    free(normArray);
    free(texcoordArray);
    free(faceArray);
    free(fileContent);
    return vertexArray;
}
