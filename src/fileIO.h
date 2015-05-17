#ifndef FILEIO_H
#define FILEIO_H

#include <GL/glew.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mesh.h"
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

// Reads a .obj file and returns an array of floats in the format:
// [pos.x, pos.y, pos.z, normal.x, normal.x, normal,y, normal.z, texcoord.x, texcoord.y,
//  pos.x, pos.y, pos.z, normal.x, normal.x, normal,y, normal.z, texcoord.x, texcoord.y,
//  ...]
GLfloat* readFromObj(const char* fileName, int *numVertices)
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
    //posArray = (GLfloat*)malloc(sizeof(GLfloat)*posCount*3);
    GLfloat *posArray = (GLfloat*)malloc(sizeof(GLfloat) * posArraySize);
    
    unsigned int normArraySize = normCount * 3;
    //normArray = (GLfloat*)malloc(sizeof(GLfloat)*normCount*3);
    GLfloat *normArray = (GLfloat*)malloc(sizeof(GLfloat) * normArraySize);

    unsigned int texcoordArraySize = texcoordCount * 2;
    //texcoordArray = (GLfloat*)malloc(sizeof(GLfloat)*texcoordCount*2);
    GLfloat *texcoordArray = (GLfloat*)malloc(sizeof(GLfloat)*texcoordArraySize);

    // Not sure how many faces there will be after making sure thay are all triangles
    // so I quadrupled the size of the faceArray
    unsigned int faceArraySize = faceCount * 3 * 3 * 4;
    //faceArray = (int*)malloc(sizeof(int)*faceCount*3*3*2 *2);
    int *faceArray = (int*)malloc(sizeof(int)*faceArraySize);

    // Iterate through fileContent and put data into the appropriate array
    unsigned int posIndex = 0, normIndex = 0, texcoordIndex = 0, faceIndex = 0;
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
                    tmpIndex = subStringNum(fileContent, buffer, readResult, tmpIndex);
                    if(j < 3 || j > (2 + i*3))
                        faceArray[faceIndex++] = atoi(buffer) - 1;
                }
            }
        } 
    }

    Mesh mesh;
    mesh.initialize(posArray, normArray, texcoordArray, faceArray, posArraySize, normArraySize, texcoordArraySize, faceIndex);

    /*
      NOTE: used to cause heap corruption.
     */
    GLfloat *vertexArray = mesh.vertexArray(numVertices);

    free(posArray);
    free(normArray);
    free(texcoordArray);
    free(faceArray);
    free(fileContent);

    return vertexArray;
}

/*
GLfloat* readFromObj(const char* fileName, int *numVertices)
{
    int posCount, normCount, texcoordCount, faceCount, vertexCount, fileSize, readResult, index;
    char *fileContent, buffer[50];;
    GLfloat *posArray, *normArray, *texcoordArray; 
    int *faceArray;
    GLfloat *vertexArray;

    // Read the file into the string fileContent
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

    // Count the number of positions, normals, texcoords, and faces in the file
    // and allocate an array for each of them.
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

    unsigned int posArraySize = posCount * 3;
    //posArray = (GLfloat*)malloc(sizeof(GLfloat)*posCount*3);
    posArray = (GLfloat*)malloc(sizeof(GLfloat) * posArraySize);
    
    unsigned int normArraySize = normCount * 3;
    //normArray = (GLfloat*)malloc(sizeof(GLfloat)*normCount*3);
    normArray = (GLfloat*)malloc(sizeof(GLfloat) * normArraySize);

    unsigned int texcoordArraySize = texcoordCount * 2;
    //texcoordArray = (GLfloat*)malloc(sizeof(GLfloat)*texcoordCount*2);
    texcoordArray = (GLfloat*)malloc(sizeof(GLfloat)*texcoordArraySize);

    // Not sure how many faces there will be after making sure thay are all triangles
    // so I quadrupled the size of the faceArray
    unsigned int faceArraySize = faceCount * 3 * 3 * 4;
    //faceArray = (int*)malloc(sizeof(int)*faceCount*3*3*2 *2);
    faceArray = (int*)malloc(sizeof(int)*faceArraySize);

    // Iterate through fileContent and put data into the appropriate array
    unsigned int posIndex = 0, normIndex = 0, texcoordIndex = 0, faceIndex = 0;
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
                    tmpIndex = subStringNum(fileContent, buffer, readResult, tmpIndex);
                    if(j < 3 || j > (2 + i*3))
                        faceArray[faceIndex++] = atoi(buffer) - 1;
                }
            }
        } 
    }

    // Construct the output array
    // (2.0f + 2.0f/3.0f) = 8/3
    vertexCount = int((float)faceIndex*(2.0f + 2.0f/3.0f));
    vertexArray = (GLfloat*)malloc(sizeof(GLfloat)*vertexCount);

    int vertexIndex = 0;
    for(unsigned int i = 0; i < faceIndex; i+=3)
    {
        // NOTE: what's with all the reversal?
        // the z component of position and normal are both reversed
        // normal used to be untouched, and the object was lit on the wrong side
        // Position
        vertexArray[vertexIndex++] = posArray[(faceArray[i])*3];
        vertexArray[vertexIndex++] = posArray[(faceArray[i])*3 + 1];
        vertexArray[vertexIndex++] = -posArray[(faceArray[i])*3 + 2];
        // Normal
        vertexArray[vertexIndex++] = normArray[(faceArray[i + 2])*3];
        vertexArray[vertexIndex++] = normArray[(faceArray[i + 2])*3 + 1];
        vertexArray[vertexIndex++] = -normArray[(faceArray[i + 2])*3 + 2];
        // Texcoord
        vertexArray[vertexIndex++] = texcoordArray[(faceArray[i + 1])*2];
        // also reversed the v component of texcoord
        vertexArray[vertexIndex++] = 1.0f - texcoordArray[(faceArray[i + 1])*2 + 1];
    }

    *numVertices = vertexIndex / 8;

    free(posArray);
    free(normArray);
    free(texcoordArray);
    free(faceArray);
    free(fileContent);

    return vertexArray;
}
*/

#endif
