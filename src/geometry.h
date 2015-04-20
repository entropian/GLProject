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
        posArray[i] = atof(buffer);
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
        normArray[i] = atof(buffer);
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


int readFrom3DS(const char* fileName, int *numVertices)
{
    int i, fileSize, numVerts, numPolygons;
    FILE *l_file;
    unsigned short l_chunk_id;
    unsigned int l_chunk_length;
    unsigned char l_char;
    unsigned short l_qty;
    unsigned short l_face_flags;
    char objName[20];
    GLfloat vertices[MAX_VERTICES];
    unsigned short polygons[MAX_POLYGONS];
    unsigned short texcoords[MAX_VERTICES];

    if((l_file = fopen(fileName, "rb")) == NULL)
        return 0;

    fseek(l_file, 0, SEEK_END);
    fileSize = ftell(l_file);
    rewind(l_file);

    while(ftell(l_file) < fileSize)
    {
        fread(&l_chunk_id, 2, 1, l_file);
        printf("ChunkID: %x\n", l_chunk_id);
        fread(&l_chunk_length, 4, 1, l_file);
        printf("ChunkLength: %x\n", l_chunk_length);

        switch(l_chunk_id)
        {
        case 0x4d4d: // Main chunk. Has no data.
            break;
        case 0x3d3d: // 3D editor chunk. Also has no data.
            break;
        case 0x4000: // Object block
            i = 0;
            do{
                fread(&l_char, 1, 1, l_file);
                objName[i] = l_char;
                i++;
            }while(l_char != '\0' && i< 20);
            break;
        case 0x4100: // empty node
            break;
        case 0x4110: // Vertices list
            fread(&l_qty, sizeof(unsigned short), 1, l_file);
            numVerts = l_qty;
            printf("Number of vertices: %d\n", l_qty);
            for(i = 0; i < l_qty; i++)
            {
                fread(&vertices[i*3], sizeof(GLfloat), 3, l_file);
            }
            break;
        case 0x4120: // Faces description
            fread(&l_qty, sizeof(unsigned short), 1, l_file);
            numPolygons = l_qty;
            printf("Number of polygons: %d\n", l_qty);
            for(i = 0; i < l_qty; i++)
            {
                fread(&(polygons[i*3]), sizeof(unsigned short), 3, l_file);
                fread(&l_face_flags, sizeof(unsigned short), 1, l_file);
            }
            break;
        case 0x4140:
            fread(&l_qty, sizeof(unsigned short), 1, l_file);
            for(i = 0; i < l_qty; i++)
                fread(&(texcoords[i*2]), sizeof(float), 2, l_file);
            break;
        default:
            fseek(l_file, l_chunk_length-6, SEEK_CUR);
        }
    }

    for(i = 0; i < numVerts; i++)
        printf("Position %f, %f, %f\n", vertices[i], vertices[i+1], vertices[i+2]);

    
    fclose(l_file);
    return 1;
}

GLfloat* readFromObj(const char* fileName, int *numVertices)
{
    int posCount, normCount, indexCount, texcoordCount, fileSize, readResult, index;
    char *fileContent, buffer[50];;
    GLfloat *posArray, *normArray, *texcoordArray; 
    int *indexArray;
    GLfloat *ret;
    
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

    //printf("%s\n", fileContent);
    posCount = normCount = texcoordCount = 0;

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
    }
    printf("posCount = %d\n", posCount);
    printf("normCount = %d\n", normCount);
    printf("texcoordCount = %d\n", texcoordCount);

    posArray = (GLfloat*)malloc(sizeof(GLfloat)*posCount*3);
    normArray = (GLfloat*)malloc(sizeof(GLfloat)*normCount*3);
    texcoordArray = (GLfloat*)malloc(sizeof(GLfloat)*texcoordCount*2);

    int posIndex = 0, normIndex = 0, texcoordIndex = 0;

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
        }
    }

    free(posArray);
    free(normArray);
    free(texcoordArray);
    free(fileContent);
    return NULL;
}
