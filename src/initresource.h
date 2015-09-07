#ifndef INITRESOURCE_H
#define INITRESOURCE_H

#if __GNUG__
#   include "SOIL/SOIL.h"
#else
#   include "SOIL.h"
#endif

#include "vec.h"
#include "fileIO.h"
#include "geometry.h"
#include "mesh.h"
#include "material.h"
#include "scenegraph.h"
#include "skybox.h"
#include "input.h"
#include "shadowshaders.h"
#include "dfshaders.h"
#include "aabb.h"

extern Skybox g_skybox;
extern Vec3 g_lightW;
extern DepthMap g_depthMap;
static bool shadow = false;
static char *TEXTURE_DIR = "../textures/";


void initGeometries(Geometries &geometries, SceneObjectEntry objEntries[], const int numObj)
{
    for(int i = 0; i < numObj; i++)
    {
        bool duplicate = false;

        for(int j = 0; j < geometries.numGroupGeo; j++)
            if(strcmp(objEntries[i].fileName, geometries.groupGeo[j]->name) == 0)
                duplicate = true;

        if(!duplicate)
        {
            Mesh mesh;
            mesh.loadOBJFile(objEntries[i].fileName);
            if(objEntries[i].calcNormal)
                mesh.computeVertexNormals();
            if(objEntries[i].calcBasis)
                mesh.computeVertexBasis();
            getGeoList(mesh, geometries.groupGeo, geometries.groupInfoList, geometries.groupLen, geometries.numGroupGeo,
                       geometries.numGroupInfo, objEntries[i].extraVertAttrib ? PNXTBD : PNX);
        }
    }
}

void loadAndSpecifyTexture(const char *fileName)
{
    int width, height;
    unsigned char *image;
    /*      
      Note: Changed SOIL_LOAD_RGB to SOIL_LOAD_RGBA so that each row of pixels in the image
      is a multiple of 4 bytes. Supposedly GPUs like data in chunks for 4 bytes. Loading images
      as RGB was crashing glTexImage2D() for certain images.       
    */
    image = SOIL_load_image(fileName, &width, &height, 0, SOIL_LOAD_RGBA);

    if(image == NULL)
        fprintf(stderr, "Failed to load %s.\n", fileName);

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB , width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SOIL_free_image_data(image);
}

bool addTextureFileName(const char fileName[40], char textureFileNames[][40], const size_t MAX_TEXTURES, size_t &tfIndex)
{
    bool duplicate = false;
    for(size_t j = 0; j < tfIndex; j++)
        if(strcmp(fileName, textureFileNames[j]) == 0)
            duplicate = true;

    if(!duplicate)
    {
        if((tfIndex + 1) >= MAX_TEXTURES)
            return false;
        strcpy(textureFileNames[tfIndex], fileName);
        tfIndex++;
    }
    return true;
}

// Initializes textures that are used in materials
int initTextures(MaterialInfo matInfoList[], const size_t matCount, char textureFileNames[][40], const size_t MAX_TEXTURES,
                 GLuint *&textureHandles)
{
    time_t startTime, endTime;
    time(&startTime);
    size_t mapCount = 0;
    for(size_t i = 0; i < matCount; i++)
    {
        if(matInfoList[i].map_Kd[0] != '\0')
            mapCount++;
        if(matInfoList[i].map_bump[0] != '\0')
            mapCount++;
        if(matInfoList[i].map_d[0] != '\0')
            mapCount++;
        if(matInfoList[i].map_spec[0] != '\0')
            mapCount++;                
    }
    
    // TODO: get rid of the redundant code in this loop
    size_t tfIndex = 0;
    bool status = true;;
    char *fileNames[4];
    for(size_t i = 0; i < matCount; i++)
    {
        fileNames[0] = matInfoList[i].map_Kd;
        fileNames[1] = matInfoList[i].map_bump;
        fileNames[2] = matInfoList[i].map_d;
        fileNames[3] = matInfoList[i].map_spec;
        for(size_t j = 0; j < 4; j++)
        {
            if(fileNames[j][0] != '\0')
            {
                status = addTextureFileName(fileNames[j], textureFileNames, MAX_TEXTURES, tfIndex);
                if(!status)
                {
                    fprintf(stderr, "Too many textures.\n");
                    break;
                }
            }
            if(!status)
                break;
        }
    }
                                                                                  
    textureHandles = (GLuint*)malloc(sizeof(GLuint)*tfIndex);
    glGenTextures(tfIndex, textureHandles);

    glActiveTexture(GL_TEXTURE0);
    for(size_t i = 0; i < tfIndex; i++)
    {
        //printf("%s\n", textureFileNames[i]);        
        char buffer[100];
        strcpy(buffer, TEXTURE_DIR);
        strcat(buffer, textureFileNames[i]);
        glBindTexture(GL_TEXTURE_2D, textureHandles[i]);
        loadAndSpecifyTexture(buffer);
        //glGenerateMipmap(GL_TEXTURE_2D);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    time(&endTime);
    double sec = difftime(endTime, startTime);
    printf("Texture loading took %f seconds.\n", sec);
    return tfIndex;
}

/*
  Find the texture handle specified by fileName in textureArray and send it to mat
  If the texture handle is not found, textureArray[i] is sent instead.
*/
void setMaterialTexture(Material *mat, const char* fileName, const char* uniformName,
                         char textureFileNames[][40], const GLuint* textureHandles, size_t numTextures)
{
    size_t i;
    for(i = 0; i < numTextures; i++)
        if(strcmp(fileName, textureFileNames[i]) == 0)
            break;
    mat->sendUniformTexture(uniformName, textureHandles[i]);    
}

/*
void initMaterials(Material *materials[], const size_t arrayLen, int &numMat, const GLuint *textureHandles)
{
    // TODO: what if two materials have the same name?
    //materials[numMat] = new Material(normalVertSrc, normalFragSrc, "Ship1Material");
    materials[numMat] = new Material(GeoPassNormalVertSrc, GeoPassNormalFragSrc, "Ship1Material");
    //Vec3 color(1.0f, 1.0f, 1.0f);
    //materials[numMat]->sendUniform3f("uColor", color);
    materials[numMat]->sendUniformTexture("diffuseMap", textureHandles[0]);
    materials[numMat]->sendUniformTexture("normalMap", textureHandles[2]);
    materials[numMat]->bindUniformBlock("UniformBlock", 0);
    ++numMat;

    materials[numMat] = new Material(GeoPassNormalVertSrc, GeoPassNormalFragSrc, "Ship2Material");
    //materials[numMat]->sendUniform3f("uColor", color);
    materials[numMat]->sendUniformTexture("diffuseMap", textureHandles[3]);
    materials[numMat]->sendUniformTexture("normalMap", textureHandles[4]);
    materials[numMat]->bindUniformBlock("UniformBlock", 0);
    ++numMat;
    
    materials[numMat] = new Material(GeoPassBasicVertSrc, GeoPassBasicFragSrc, "TeapotMaterial");
    //materials[numMat]->sendUniform3f("uColor", color);
    materials[numMat]->sendUniformTexture("diffuseMap", textureHandles[1]);
    materials[numMat]->bindUniformBlock("UniformBlock", 0);
    ++numMat;
    
    
    materials[numMat] = new Material(GeoPassBasicVertSrc, GeoPassBasicFragSrc, "CubeMaterial");
    //g_cubeMaterial = new Material(basicVertSrc, diffuseFragSrc, exampleGeoSrc, "CubeMaterial");    
    //g_cubeMaterial->sendUniform3f("uColor", Vec3(1.0f, 1.0f, 1.0f));
    materials[numMat]->bindUniformBlock("UniformBlock", 0);
    ++numMat;

    // TODO: change g_skybox?
    materials[numMat] = new Material(cubemapReflectionVertSrc, cubemapReflectionFragSrc, "CubemapReflectionMat");
    materials[numMat]->sendUniformCubemap("uCubemap", g_skybox.cubemap);
    materials[numMat]->bindUniformBlock("UniformBlock", 0);    
    ++numMat;

    materials[numMat] = new Material(showNormalVertSrc, basicFragSrc, showNormalGeoSrc, "ShowNormalMaterial");
    materials[numMat]->bindUniformBlock("UniformBlock", 0);
    ++numMat;
}
*/

void initMTLMaterials(MaterialInfo *matInfoList, const size_t matCount, Material *MTLMaterials[], int &numMTLMat,
                      char textureFileNames[][40], const GLuint *textureHandles, const int numTextures)
{
    enum ShaderFlag
    {
        NONE = 0,
        DIFFUSE = 1,
        NORMAL = 2,
        SPECULAR = 4,
        ALPHA = 8
    };

    // Populate materials with data in matInfoList
    for(size_t i = 0; i < matCount; i++)
    {
        ShaderFlag sf = NONE;
        if(matInfoList[i].map_Kd[0] != '\0')
            sf = static_cast<ShaderFlag>(static_cast<int>(sf) | static_cast<int>(DIFFUSE));        
        if(matInfoList[i].map_spec[0] != '\0')
            sf = static_cast<ShaderFlag>(static_cast<int>(sf) | static_cast<int>(SPECULAR));
        //if(matInfoList[i].map_bump[0] != '\0')
        if(strstr(matInfoList[i].map_bump, "ddn"))
            sf = static_cast<ShaderFlag>(static_cast<int>(sf) | static_cast<int>(NORMAL));
        if(matInfoList[i].map_d[0] != '\0')
            sf = static_cast<ShaderFlag>(static_cast<int>(sf) | static_cast<int>(ALPHA));
        switch(sf)
        {
        case NONE:
                MTLMaterials[i] = new Material(GeoPassBasicVertSrc, GeoPassOBJFragSrc, matInfoList[i].name);
                break;
        case DIFFUSE:
                MTLMaterials[i] = new Material(GeoPassBasicVertSrc, GeoPassOBJDFragSrc, matInfoList[i].name);
            break;
        case DIFFUSE|NORMAL:
                MTLMaterials[i] = new Material(GeoPassNormalVertSrc, GeoPassOBJNDFragSrc, matInfoList[i].name);
            break;
        case DIFFUSE|SPECULAR:
                MTLMaterials[i] = new Material(GeoPassBasicVertSrc, GeoPassOBJDSFragSrc, matInfoList[i].name);                
            break;
        case DIFFUSE|NORMAL|SPECULAR:
                MTLMaterials[i] = new Material(GeoPassNormalVertSrc, GeoPassOBJNDSFragSrc, matInfoList[i].name);                
            break;
        case DIFFUSE|ALPHA:
                MTLMaterials[i] = new Material(basicVertSrc, OBJAlphaFragSrc, matInfoList[i].name);
            break;
        case DIFFUSE|NORMAL|ALPHA:
                MTLMaterials[i] = new Material(GeoPassNormalVertSrc, GeoPassOBJNADFragSrc, matInfoList[i].name);
            break;
        case DIFFUSE|SPECULAR|ALPHA:
                MTLMaterials[i] = new Material(GeoPassBasicVertSrc, GeoPassOBJADSFragSrc, matInfoList[i].name);
            break;
        case DIFFUSE|NORMAL|SPECULAR|ALPHA:
                MTLMaterials[i] = new Material(GeoPassBasicVertSrc, GeoPassOBJNADSFragSrc, matInfoList[i].name);                
            break;
        }

        //MTLMaterials[i]->sendUniform3f("Ka", matInfoList[i].Ka);
        MTLMaterials[i]->sendUniform3f("Kd", matInfoList[i].Kd);
        //MTLMaterials[i]->sendUniform3f("Ks", matInfoList[i].Ks);
        MTLMaterials[i]->sendUniform1f("Ns", matInfoList[i].Ns);
        MTLMaterials[i]->bindUniformBlock("UniformBlock", 0);

        // Diffuse map
        if(sf & DIFFUSE)
            setMaterialTexture(MTLMaterials[i], matInfoList[i].map_Kd, "diffuseMap", textureFileNames, textureHandles,
                               numTextures);
        // Normal map
        if(sf & NORMAL)
            setMaterialTexture(MTLMaterials[i], matInfoList[i].map_bump, "normalMap", textureFileNames, textureHandles,
                               numTextures);
        // Alpha map
        if(sf & ALPHA)
            setMaterialTexture(MTLMaterials[i], matInfoList[i].map_d, "alphaMap", textureFileNames, textureHandles,
                               numTextures);
        // Specular map
        if(sf & SPECULAR)
            setMaterialTexture(MTLMaterials[i], matInfoList[i].map_spec, "specularMap", textureFileNames, textureHandles,
                               numTextures);
    }
    numMTLMat += matCount;
}

//Returns the element in singlesArray that has the name geoName and NULL if the element isn't found.
Geometry* getSingleGeoFromArray(Geometry *singlesArray[], const int numSingleGeo, const char *geoName)
{
    int i;
    for(i = 0; i < numSingleGeo; i++)
        if(strcmp(singlesArray[i]->name, geoName) == 0)
            break;
    if(i < numSingleGeo)
        return singlesArray[i];
    fprintf(stderr, "Geometry %s not found.\n", geoName);
    return NULL;
}

//Returns the element in infoList that has the name geoName and NULL if the element isn't found.
int getGroupInfoFromArray(const GeoGroupInfo infoList[], const int numGroupInfo, const char* groupName)
{
    int i = 0;
    for(i = 0; i < numGroupInfo; i++)
    {
        printf("%s\n", infoList[i].name);
        if(strcmp(infoList[i].name, groupName) == 0)            
            break;
    }
    if(i < numGroupInfo)
        return i;
    fprintf(stderr, "Geometry group %s not found.\n", groupName);
    return -1;
}

//Returns the element in materials that has the name geoName and NULL if the element isn't found.
Material* getMaterialFromArray(Material *materials[], const int numMat, const char* matName)
{
    int i;
    for(i = 0; i < numMat; i++)
        if(strcmp(materials[i]->getName(), matName) == 0)
           break;
    if(i < numMat)
        return materials[i];
    return NULL;
}

// TODO: some of the arrays in the parameters don't have length specified. Also sort out the references and pointers
void initScene(TransformNode *rootNode, Geometries &geometries, MaterialInfo matInfoList[], const size_t MAX_MATERIALS,
               Material *MTLMaterials[], int &numMTLMat, GLuint *&textureHandles, int &numTextures, char textureFileNames[][40], 
               const size_t MAX_TEXTURES, BaseGeometryNode *baseGeoNodes[], const int bgnArrayLen, int &numbgn)    
{
    SceneObjectEntry objEntries[50];
    initSceneObjectEntries(objEntries, 50);
    int numObj = loadSceneFile(objEntries, 50, "scene2.txt");

    initGeometries(geometries, objEntries, numObj);

    char MTLFileNames[30][30];    
    for(int i = 0; i < numObj; i++)
    {
        strcpy(MTLFileNames[i], objEntries[i].MTLFileName);
    }
    size_t MTLMatCount = loadMTLFiles(matInfoList, MAX_MATERIALS, MTLFileNames, numObj);
    numTextures = initTextures(matInfoList, MTLMatCount, textureFileNames, MAX_TEXTURES, textureHandles);
    initMTLMaterials(matInfoList, MTLMatCount, MTLMaterials, numMTLMat, textureFileNames, textureHandles, numTextures);

    for(int i = 0; i < numMTLMat; i++)        
        MTLMaterials[i]->sendUniformTexture("shadowMap", g_depthMap.depthMap);    

    for(int i = 0; i < numObj; i++)
    {
        MultiGeometryNode *mgn;
        RigTForm modelRbt(objEntries[i].position);
        int index = getGroupInfoFromArray(geometries.groupInfoList, geometries.numGroupInfo, objEntries[i].fileName);
        if(index != -1)
        {
            mgn = new MultiGeometryNode(geometries.groupGeo, geometries.groupInfoList[index], MTLMaterials,
                                        numMTLMat, modelRbt, true);
            mgn->setScaleFactor(objEntries[i].scaleFactors);
        }
        baseGeoNodes[numbgn++] = mgn;
        rootNode->addChild(mgn);        
    }
}

#endif
