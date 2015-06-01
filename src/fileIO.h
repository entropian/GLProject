#ifndef FILEIO_H
#define FILEIO_H

#include <GL/glew.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "vec.h"


// returns the index of the character immediately after the substring.
// returns -1 if no new substring
static int subStringAlpha(const char* fileContent, char buffer[], size_t fileSize, size_t index)
{
    size_t i, j;
    //for(i = index; (fileContent[i] < 'A' || fileContent[i] > 'Z') && (fileContent[i] < 'a' || fileContent[i] > 'z'); i++)
    for(i = index; isalpha(fileContent[i]) == 0; i++)
    {
        if(i >= fileSize)
            return -1;
    }

    //for(j = 0; (fileContent[j+i] >= 'A' && fileContent[j+i] <= 'Z') || (fileContent[j+i] >= 'a' && fileContent[j+i] <= 'z'); j++)
    for(j = 0; isalpha(fileContent[j+i]) || fileContent[j+i] == '_';j++)
        buffer[j] = fileContent[j+i];

    buffer[j] = '\0';

    return int(j + i);
}

// returns the index of the character immediately after the substring.
// returns -1 if no new substring
static int subStringNum(const char* fileContent, char buffer[], size_t fileSize, size_t index)
{
    size_t i, j;
    //for(i = index; (fileContent[i] < '0' || fileContent[i] > '9') && fileContent[i] != '-'; i++)
    for(i = index; isdigit(fileContent[i]) == 0 && fileContent[i] != '-'; i++)
    {
        if(i >= fileSize)
            return -1;
    }

    for(j = 0;
        (isdigit(fileContent[j+i]) != 0 || fileContent[j+i] == '.') || fileContent[j+i] == '-';
        j++)
        buffer[j] = fileContent[j+i];

    buffer[j] = '\0';
    return int(j + i);
}

static int getMaterialName(const char *fileContent, char buffer[], size_t fileSize, size_t index)
{
    size_t i, j;
    //for(i = index; (fileContent[i] < 'A' || fileContent[i] > 'Z') && (fileContent[i] < 'a' || fileContent[i] > 'z'); i++)
    for(i = index; isalnum(fileContent[i]) == 0; i++)
    {
        if(i >= fileSize)
            return -1;
    }

    for(j = 0; fileContent[j+i] != '\n'; j++)
        buffer[j] = fileContent[j+i];
    buffer[j] = '\0';

    return int(j + i);
}

static size_t readFileIntoString(const char *fileName, char **fileContent)
{
    // Read the file into the string fileContent
    FILE *fp = fopen(fileName, "rb");
    if(fp == NULL)
    {
        fprintf(stderr, "Cannot open file.");
        exit(0);
    }
    
    fseek(fp, 0L, SEEK_END);
    size_t fileSize = ftell(fp);
    *fileContent = (char*)malloc(sizeof(char) * fileSize + 1);
    rewind(fp);
    //printf("File size = %d\n", fileSize);

    size_t readResult = fread((void*)(*fileContent), 1, fileSize, fp);    
    if(readResult != fileSize)
    {
        fprintf(stderr, "Reading error.\n");
        exit(1);
    }
    fclose(fp);
    (*fileContent)[readResult] = '\0';

    return readResult;
}

struct MaterialInfo
{    
    char name[30];
    Vec3 Ka, Kd, Ks, Ke; // Ambient color, diffuse color, specular color, and emmisive color
    float Ns = 0.0f, Ni = 0.0f;         // Specular exponent and index of reflection
    int illum = 0;            // Illumination mode (0=constant, 1=diffuse, 2=diffuse+specular...)
    char map_Ka[30];
    char map_Kd[30];
    char map_bump[30];
};

static size_t parseMtlFile(const char *fileName, MaterialInfo *&infoList)
{
    char *fileContent;
    size_t readResult = readFileIntoString(fileName, &fileContent);

    char buffer[50];
    size_t matCount = 0;
    int index = 0;
    while(index != -1)
    {
        index = subStringAlpha(fileContent, buffer, readResult, index);
        if(strcmp(buffer, "newmtl") == 0)
            matCount++;
    }

    infoList = (MaterialInfo*)malloc(sizeof(MaterialInfo)*matCount);

    for(size_t i = 0; i < matCount; i++)
    {
        infoList[i].name[0] = '\0';
        infoList[i].map_Ka[0] = '\0';
        infoList[i].map_Kd[0] = '\0';
        infoList[i].map_bump[0] = '\0';
    }

    size_t infoIndex = 0;
    index = 0;
    while(index != -1)
    {
        index = subStringAlpha(fileContent, buffer, readResult, index);
        if(strcmp(buffer, "newmtl") == 0)
        {
            index = getMaterialName(fileContent, buffer, readResult, index);
            strcpy(infoList[infoIndex].name, buffer);
            infoIndex++;
        }else if(strcmp(buffer, "Ka") == 0)
        {
            index = subStringNum(fileContent, buffer, readResult, index);
            infoList[infoIndex-1].Ka[0] = (float)atof(buffer);
            index = subStringNum(fileContent, buffer, readResult, index);
            infoList[infoIndex-1].Ka[1] = (float)atof(buffer);
            index = subStringNum(fileContent, buffer, readResult, index);
            infoList[infoIndex-1].Ka[2] = (float)atof(buffer);            
        }else if(strcmp(buffer, "Kd") == 0)
        {
            index = subStringNum(fileContent, buffer, readResult, index);
            infoList[infoIndex-1].Kd[0] = (float)atof(buffer);
            index = subStringNum(fileContent, buffer, readResult, index);
            infoList[infoIndex-1].Kd[1] = (float)atof(buffer);
            index = subStringNum(fileContent, buffer, readResult, index);
            infoList[infoIndex-1].Kd[2] = (float)atof(buffer);            
        }else if(strcmp(buffer, "Ks") == 0)
        {
            index = subStringNum(fileContent, buffer, readResult, index);
            infoList[infoIndex-1].Ks[0] = (float)atof(buffer);
            index = subStringNum(fileContent, buffer, readResult, index);
            infoList[infoIndex-1].Ks[1] = (float)atof(buffer);
            index = subStringNum(fileContent, buffer, readResult, index);
            infoList[infoIndex-1].Ks[2] = (float)atof(buffer);
        }else if(strcmp(buffer, "Ke") == 0)
        {
            index = subStringNum(fileContent, buffer, readResult, index);
            infoList[infoIndex-1].Ke[0] = (float)atof(buffer);
            index = subStringNum(fileContent, buffer, readResult, index);
            infoList[infoIndex-1].Ke[1] = (float)atof(buffer);
            index = subStringNum(fileContent, buffer, readResult, index);
            infoList[infoIndex-1].Ke[2] = (float)atof(buffer);            
        }else if(strcmp(buffer, "Ns") == 0)
        {
            index = subStringNum(fileContent, buffer, readResult, index);
            infoList[infoIndex-1].Ns = (float)atof(buffer);
        }else if(strcmp(buffer, "Ni") == 0)
        {
            index = subStringNum(fileContent, buffer, readResult, index);
            infoList[infoIndex-1].Ni = atof(buffer);
        }else if(strcmp(buffer, "illum") == 0)
        {
            index = subStringNum(fileContent, buffer, readResult, index);
            infoList[infoIndex-1].illum = atoi(buffer);                                                
        }else if(strcmp(buffer, "map_Kd") == 0)
        {
            index = getMaterialName(fileContent, buffer, readResult, index);
            strcpy(infoList[infoIndex-1].map_Kd, buffer);
        }else if(strcmp(buffer, "map_Ka") == 0)
        {
            index = getMaterialName(fileContent, buffer, readResult, index);
            strcpy(infoList[infoIndex-1].map_Ka, buffer);
        }else if(strcmp(buffer, "map_bump") == 0)
        {
            index = getMaterialName(fileContent, buffer, readResult, index);
            strcpy(infoList[infoIndex-1].map_bump, buffer);
        }            

    }

    return matCount;
}


static GLfloat* readFromCollada(const char* fileName, int *numVertices)
{


    char buffer[50], *fileContent;
    int i, posCount, normCount, indexCount, fileSize, index;
    float *posArray, *normArray, *vertexArray;
    int *indexArray;
    
    /*
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
    */
    size_t readResult = readFileIntoString(fileName, &fileContent);
    
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


#endif
