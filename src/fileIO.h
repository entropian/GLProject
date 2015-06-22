#ifndef FILEIO_H
#define FILEIO_H

#include <GL/glew.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
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
        fprintf(stderr, "Cannot open file %s.\n", fileName);
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

static size_t parseMTLFile(MaterialInfo *infoList, const size_t infoListSize, const char *fileName)
{

    char *fileContent;
    size_t readResult = readFileIntoString(fileName, &fileContent);

    if(readResult == 0)
    {
        fprintf(stderr, "Read nothing from %s\n", fileName);
        return 0;
    }

    char buffer[50];
    size_t matCount = 0;
    int index = 0;
    while(index != -1)
    {
        index = subStringAlpha(fileContent, buffer, readResult, index);
        if(strcmp(buffer, "newmtl") == 0)
            matCount++;
    }

    if(matCount > infoListSize)
    {
        fprintf(stderr, "Too many materials in %s.\n", fileName);
        return 0;
    }
    //infoList = (MaterialInfo*)malloc(sizeof(MaterialInfo)*matCount);

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
            index = getMaterialName(fileContent, infoList[infoIndex].name, readResult, index);
            //strcpy(infoList[infoIndex].name, buffer);
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
            infoList[infoIndex-1].Ni = (float)atof(buffer);
        }else if(strcmp(buffer, "illum") == 0)
        {
            index = subStringNum(fileContent, buffer, readResult, index);
            infoList[infoIndex-1].illum = atoi(buffer);                                                
        }else if(strcmp(buffer, "map_Kd") == 0)
        {
            index = getMaterialName(fileContent, infoList[infoIndex-1].map_Kd, readResult, index);
            //strcpy(infoList[infoIndex-1].map_Kd, buffer);
        }else if(strcmp(buffer, "map_Ka") == 0)
        {
            index = getMaterialName(fileContent, infoList[infoIndex-1].map_Ka, readResult, index);
            //strcpy(infoList[infoIndex-1].map_Ka, buffer);
        }else if(strcmp(buffer, "map_bump") == 0)
        {
            index = getMaterialName(fileContent, infoList[infoIndex-1].map_bump, readResult, index);
            //strcpy(infoList[infoIndex-1].map_bump, buffer);
        }            
    }

    free(fileContent);    
    return matCount;
}

struct OBJData
{
    GLfloat *positions = NULL, *normals = NULL, *texcoords = NULL;
    int *faces = NULL;
    size_t *groupIndices;;
    char **mtlNames = NULL;
    size_t numPositions = 0, numNormals = 0, numTexcoords = 0, numFaces = 0, numGroups = 0;    
};

static void extractOBJData(const char *fileContent, const size_t fileSize, OBJData *objData)
{
    char buffer[20];
    size_t posIndex = 0, normIndex = 0, texcoordIndex = 0, faceIndex = 0, groupIndex = 0;
    int index = 0;
    while(index != -1)
    {
        index = subStringAlpha(fileContent, buffer, fileSize, index);
        if(strcmp(buffer, "v") == 0)
        {
            index = subStringNum(fileContent, buffer, fileSize, index);
            objData->positions[posIndex++] = (GLfloat)atof(buffer);
            index = subStringNum(fileContent, buffer, fileSize, index);
            objData->positions[posIndex++] = (GLfloat)atof(buffer);
            index = subStringNum(fileContent, buffer, fileSize, index);
            // Reversing Z
            objData->positions[posIndex++] = -(GLfloat)atof(buffer);
        }else if(strcmp(buffer, "vt") == 0)
        {
            index = subStringNum(fileContent, buffer, fileSize, index);            
            objData->texcoords[texcoordIndex++] = (GLfloat)atof(buffer);
            index = subStringNum(fileContent, buffer, fileSize, index);
            // Reversing V
            objData->texcoords[texcoordIndex++] = 1.0f - (GLfloat)atof(buffer);
        }else if(strcmp(buffer, "vn") == 0)
        {
            index = subStringNum(fileContent, buffer, fileSize, index);
            objData->normals[normIndex++] = (GLfloat)atof(buffer);
            index = subStringNum(fileContent, buffer, fileSize, index);
            objData->normals[normIndex++] = (GLfloat)atof(buffer);            
            index = subStringNum(fileContent, buffer, fileSize, index);
            objData->normals[normIndex++] = (GLfloat)atof(buffer);            
        }else if(strcmp(buffer, "f") == 0)
        {
            int slashCount = 0;
 
            // Count the number of slashes in the line to determine what polygon the face is.
            for(int i = index; fileContent[i] != '\n'; i++)
            {
                if(fileContent[i] == '/')
                    slashCount++;
            }

            if(objData->numNormals > 0)
            {
                int numFaceVerts = slashCount / 2;
                int tmpIndex;
                for(int i = 0; i < (numFaceVerts - 2); i++)
                {
                    tmpIndex = index;
                    for(int j = 0; j < (9 + i*3); j++)
                    {
                        tmpIndex = subStringNum(fileContent, buffer, fileSize, tmpIndex);
                        if(j < 3 || j > (2 + i*3))
                            objData->faces[faceIndex++] = atoi(buffer) - 1;
                    }
                }
            }else
            {
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
                            objData->faces[faceIndex++] = atoi(buffer) - 1;
                    }
                }
            }
        }else if(strcmp(buffer, "g") == 0)
        {

            objData->groupIndices[groupIndex] = faceIndex / (3 * 3); // 3 indices per vertex and 3 vertices per face

            while(fileContent[index++] != '\n'); // Skip over group name
            // Assuming "usemtl blah_material" is the next line
            index = subStringAlpha(fileContent, buffer, fileSize, index); // Skip over "usemtl"

            index = getMaterialName(fileContent, buffer, fileSize, index);

            strcpy(objData->mtlNames[groupIndex++], buffer);
        }
    }
    objData->numFaces = faceIndex;
}

static void parseOBJFile(const char *fileName, OBJData *objData)
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

    // Allocate memory for objData members
    {
        objData->numPositions = posCount * 3;
        objData->positions = (GLfloat*)malloc(sizeof(GLfloat) * objData->numPositions);

        if(normCount > 0)
        {
            objData->numNormals = normCount * 3;
            objData->normals = (GLfloat*)malloc(sizeof(GLfloat) * objData->numNormals);
        }

        objData->numTexcoords = texcoordCount * 2;
        objData->texcoords = (GLfloat*)malloc(sizeof(GLfloat) * objData->numTexcoords);

        // Not sure how many faces there will be after making sure thay are all triangles
        // so I quadrupled the size of the faceArray
        objData->numFaces = faceCount * 3 * 3 * 4;
        //faceArray = (int*)malloc(sizeof(int)*faceCount*3*3*2 *2);
        objData->faces = (int*)malloc(sizeof(int) * objData->numFaces);

        if(groupCount > 0)
        {
            objData->numGroups = groupCount;
            objData->groupIndices = (size_t*)malloc(sizeof(size_t)*groupCount);
            objData->mtlNames = (char**)malloc(sizeof(char*)*groupCount);
            for(size_t i = 0; i < groupCount; i++)
                objData->mtlNames[i] = (char*)malloc(sizeof(char*)*20);
        }
    }

    //size_t faceIndex = extractOBJData(fileContent, readResult, objData);
    extractOBJData(fileContent, readResult, objData);
    
    free(fileContent);
}

/*
  static size_t parseMTLFile(const char *fileName, MaterialInfo *&infoList)
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
            infoList[infoIndex-1].Ni = (float)atof(buffer);
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
*/


static GLfloat* readFromCollada(const char* fileName, int *numVertices)
{


    char buffer[50], *fileContent;
    int i, posCount, normCount, indexCount, index;
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
