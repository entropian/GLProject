#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

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
static int subStringAlpha(const char* fileContent, char buffer[], int index)
{
    int i, j;
    for(i = index; (fileContent[i] < 'A' || fileContent[i] > 'Z') && (fileContent[i] < 'a' || fileContent[i] > 'z'); i++)
    {}

    for(j = 0; (fileContent[j+i] >= 'A' && fileContent[j+i] <= 'Z') || (fileContent[j+i] >= 'a' && fileContent[j+i] <= 'z'); j++)
        buffer[j] = fileContent[j+i];

    buffer[j] = '\0';

    return j + i;
}

static int subStringNum(const char* fileContent, char buffer[], int index)
{
    int i, j;
    for(i = index; fileContent[i] < '0' || fileContent[i] > '9'; i++)
    {}

    for(j = 0; (fileContent[j+i] >= '0' && fileContent[j+i] <= '9') || fileContent[i] == '.'; j++)
        buffer[j] = fileContent[j+i];

    buffer[j] = '\0';
    return j + 1;
}

/*
//void readFromCollada(const char* fileName, Geometry *geometry)
GLfloat* readFromCollada(const char* fileName, int *numVertices)
{


    char buffer[50], *fileContent;
    char c;
    int i, j, posCount, normCount, indexCount, fileSize, readResult, index;
    float *posArray, *normArray, *vertexArray;
    int *indexArray;
    char *token;
    
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
    
    // New stuff
    index = 0;
    while(strcmp(buffer, "count") != 0)
        index = subStringAlpha(fileContent, buffer, index);

    index = subStringNum(fileContent, buffer, index);
    posCount = atoi(buffer);
    posArray = (float*)malloc(sizeof(float) * posCount);
    for(i = 0; i < posCount; i++)
    {
        index = subStringNum(fileContent, buffer, index);
        printf("%s\n", buffer);
        posArray[i] = atof(buffer);
    }

    for(i = 0; i < posCount; i++)
        //printf("%f\n", posArray[i]);


    // Normal stuff
    fp = fopen(fileName, "r");
    fscanf_s(fp, "%s", buffer, sizeof(buffer));
    while(strcmp(buffer, "<mesh>") != 0)
        fscanf_s(fp, "%s", buffer, sizeof(buffer));
    
    
    fscanf_s(fp, "%s", buffer, sizeof(buffer));
    fscanf_s(fp, "%s", buffer, sizeof(buffer));
    fscanf_s(fp, "%s", buffer, sizeof(buffer));
    fscanf_s(fp, "%s", buffer, sizeof(buffer));

    c = fgetc(fp);
    while(c != '"')
        c = fgetc(fp);

    i = 0;
    c = fgetc(fp);
    while(c != '"')
    {
        buffer[i++] = c;
        c = fgetc(fp);
    }
    buffer[i++] = '\0';

    // Read in the array of positions
    posCount = atoi(buffer);
    printf("posCount = %d\n", posCount);
    posArray = (float*)malloc(sizeof(float) * posCount);
    
    fgetc(fp);
    for(i = 0; i < posCount; i++)
        fscanf(fp, "%f", &(posArray[i]));

    // Read in the array of normals
    while(strcmp(buffer, "<source") != 0)
        fscanf_s(fp, "%s", buffer, sizeof(buffer));

    fscanf_s(fp, "%s", buffer, sizeof(buffer));
    fscanf_s(fp, "%s", buffer, sizeof(buffer));
    fscanf_s(fp, "%s", buffer, sizeof(buffer));

    c = fgetc(fp);
    while(c != '"')
        c = fgetc(fp);

    i = 0;
    c = fgetc(fp);
    while(c != '"')
    {
        buffer[i++] = c;
        c = fgetc(fp);
    }
    buffer[i++] = '\0';
    normCount = atoi(buffer);
    printf("normCount = %d\n", normCount);
    normArray = (float*)malloc(sizeof(float) * normCount);
    
    fgetc(fp);
    for(i = 0; i < normCount; i++)
        fscanf(fp, "%f", &(normArray[i]));

    // Read in the array of indices
    while(strcmp(buffer, "</vcount>") != 0)
        fscanf_s(fp, "%s", buffer, sizeof(buffer));

    while(c != '>')
        c = fgetc(fp);

    indexCount = normCount * 2;
    indexArray = (int*)malloc(sizeof(int) * indexCount);
    for(i = 0; i < indexCount; i++)
        fscanf(fp, "%d", &(indexArray[i]));

    int max = 0;
    for(i = 0; i < indexCount; i+=2)
    {
        if(indexArray[i] > max)
            max = indexArray[i];
    }
    printf("%d\n", max);
        
    // Stuff positions and normals into the final vertex attribute array
    // multiplied by 4 to account for texcoord
    vertexArray = (float*)malloc(sizeof(float) * indexCount * 4);
    
    for(i = 0; i < indexCount/2; i++)
    {
        // position
        vertexArray[i*8] = posArray[indexArray[i*2]*3];
        vertexArray[i*8 + 1] = posArray[indexArray[i*2]*3 + 1];
        vertexArray[i*8 + 2] = posArray[indexArray[i*2]*3 + 2];
        // normal
        vertexArray[i*8 + 3] = normArray[indexArray[i*2 + 1]*3];
        vertexArray[i*8 + 4] = normArray[indexArray[i*2 + 1]*3 + 1];
        vertexArray[i*8 + 5] = normArray[indexArray[i*2 + 1]*3 + 2];
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
*/

GLfloat* readFromCollada(const char* fileName, int *numVertices)
{


    char buffer[50];
    char c;
    int i, posCount, normCount, indexCount;
    float *posArray, *normArray, *vertexArray;
    int *indexArray;
    
    // Determine the size of the file
    FILE *fp = fopen(fileName, "r");
    if(fp == NULL)
    {
        fprintf(stderr, "Cannot open file.");
        exit(0);
    }
    fscanf_s(fp, "%s", buffer, sizeof(buffer));
    while(strcmp(buffer, "<mesh>") != 0)
        fscanf_s(fp, "%s", buffer, sizeof(buffer));
    
    
    fscanf_s(fp, "%s", buffer, sizeof(buffer));
    fscanf_s(fp, "%s", buffer, sizeof(buffer));
    fscanf_s(fp, "%s", buffer, sizeof(buffer));
    fscanf_s(fp, "%s", buffer, sizeof(buffer));

    c = fgetc(fp);
    while(c != '"')
        c = fgetc(fp);

    i = 0;
    c = fgetc(fp);
    while(c != '"')
    {
        buffer[i++] = c;
        c = fgetc(fp);
    }
    buffer[i++] = '\0';

    // Read in the array of positions
    posCount = atoi(buffer);
    printf("posCount = %d\n", posCount);
    posArray = (float*)malloc(sizeof(float) * posCount);
    
    fgetc(fp);
    for(i = 0; i < posCount; i++)
        fscanf(fp, "%f", &(posArray[i]));

    // Read in the array of normals
    while(strcmp(buffer, "<source") != 0)
        fscanf_s(fp, "%s", buffer, sizeof(buffer));

    fscanf_s(fp, "%s", buffer, sizeof(buffer));
    fscanf_s(fp, "%s", buffer, sizeof(buffer));
    fscanf_s(fp, "%s", buffer, sizeof(buffer));

    c = fgetc(fp);
    while(c != '"')
        c = fgetc(fp);

    i = 0;
    c = fgetc(fp);
    while(c != '"')
    {
        buffer[i++] = c;
        c = fgetc(fp);
    }
    buffer[i++] = '\0';
    normCount = atoi(buffer);
    printf("normCount = %d\n", normCount);
    normArray = (float*)malloc(sizeof(float) * normCount);
    
    fgetc(fp);
    for(i = 0; i < normCount; i++)
        fscanf(fp, "%f", &(normArray[i]));

    // Read in the array of indices
    while(strcmp(buffer, "</vcount>") != 0)
        fscanf_s(fp, "%s", buffer, sizeof(buffer));

    while(c != '>')
        c = fgetc(fp);

    indexCount = normCount * 2;
    indexArray = (int*)malloc(sizeof(int) * indexCount);
    for(i = 0; i < indexCount; i++)
        fscanf(fp, "%d", &(indexArray[i]));

    int max = 0;
    for(i = 0; i < indexCount; i+=2)
    {
        if(indexArray[i] > max)
            max = indexArray[i];
    }
    printf("%d\n", max);
        
    // Stuff positions and normals into the final vertex attribute array
    // multiplied by 4 to account for texcoord
    vertexArray = (float*)malloc(sizeof(float) * indexCount * 4);
    
    for(i = 0; i < indexCount/2; i++)
    {
        // position
        vertexArray[i*8] = posArray[indexArray[i*2]*3];
        vertexArray[i*8 + 1] = posArray[indexArray[i*2]*3 + 1];
        vertexArray[i*8 + 2] = posArray[indexArray[i*2]*3 + 2];
        // normal
        vertexArray[i*8 + 3] = normArray[indexArray[i*2 + 1]*3];
        vertexArray[i*8 + 4] = normArray[indexArray[i*2 + 1]*3 + 1];
        vertexArray[i*8 + 5] = normArray[indexArray[i*2 + 1]*3 + 2];
        // texcoord
        vertexArray[i*8 + 6] = 0.0f;
        vertexArray[i*8 + 7] = 0.0f;
    }

    free(posArray);
    free(normArray);
    free(indexArray);

    *numVertices = indexCount/2;
    return vertexArray;
}

