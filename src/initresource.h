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

extern Skybox g_skybox;
extern Vec3 g_lightW;

// TODO: check for array length
void initSingleGeometries(Geometry *singlesArray[], int &numSingleGeo)
{
    Mesh ship1Mesh;
    ship1Mesh.loadOBJFile("Ship.obj");
    ship1Mesh.computeVertexBasis();
    //g_ship1 = ship1Mesh.produceGeometry(PNXTBD);
    singlesArray[numSingleGeo++] = ship1Mesh.produceGeometry(PNXTBD);

    Mesh ship2Mesh;
    ship2Mesh.loadOBJFile("Ship2.obj");
    ship2Mesh.computeVertexBasis();
    //g_ship2 = ship2Mesh.produceGeometry(PNXTBD);
    singlesArray[numSingleGeo++] = ship2Mesh.produceGeometry(PNXTBD);

    Mesh cubeMesh;
    cubeMesh.loadOBJFile("cube.obj");
    //g_cube = cubeMesh.produceGeometry(PNX);
    singlesArray[numSingleGeo++] = cubeMesh.produceGeometry(PNX);

    Mesh teapotMesh;
    teapotMesh.loadOBJFile("teapot.obj");
    teapotMesh.computeVertexNormals();
    //g_teapot = teapotMesh.produceGeometry(PNX);
    singlesArray[numSingleGeo++] = teapotMesh.produceGeometry(PNX);
}

void initGroupGeometries(Geometry *groupArray[], GeoGroupInfo infoArray[], const size_t arrayLen, int &numGroupGeo,
    int &numGroupInfo)
{
    Mesh sponzaMesh;
    sponzaMesh.loadOBJFile("sponza.obj");
    sponzaMesh.computeVertexNormals();
    getGeoList(sponzaMesh, groupArray, infoArray, arrayLen, numGroupGeo, numGroupInfo, PNX);    
    //getGeoList(sponzaMesh, g_geometryGroups, g_groupInfoList, MAX_GEOMETRY_GROUPS, g_groupSize, g_groupInfoSize, PNXTBD);

    Mesh crysponzaMesh;
    crysponzaMesh.loadOBJFile("crysponza.obj");
    crysponzaMesh.computeVertexBasis();    
    //getGeoList(crysponzaMesh, g_geometryGroups, g_groupInfoList, MAX_GEOMETRY_GROUPS, g_groupSize, g_groupInfoSize, PNX);
    getGeoList(crysponzaMesh, groupArray, infoArray, arrayLen, numGroupGeo, numGroupInfo, PNXTBD);   
}

void initGeometries(Geometries &geometries)
{
    initSingleGeometries(geometries.singleGeo, geometries.numSingleGeo);
    initGroupGeometries(geometries.groupGeo, geometries.groupInfoList, geometries.groupLen,
                        geometries.numGroupGeo, geometries.numGroupInfo);
}

void loadAndSpecifyTexture(const char *fileName)
{
    int width, height;
    unsigned char *image;
    /*      
      Note:Changed SOIL_LOAD_RGB to SOIL_LOAD_RGBA so that each row of pixels in the image
      is a multiple of 4 bytes. Supposedly GPUs like data in chunks for 4 bytes. Loading images
      as RGB was crashing glTexImage2D() for certain images.       
    */
    image = SOIL_load_image(fileName, &width, &height, 0, SOIL_LOAD_RGBA);

    if(image == NULL)
        fprintf(stderr, "Failed to load %s.\n", fileName);

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB , width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SOIL_free_image_data(image);
}

// Initializes textures that are used in materials
int initTextures(MaterialInfo matInfoList[], const size_t matCount, char **&textureFileNames, char *nonMTLTextures[], const int numNonMTL, GLuint *&textureHandles)
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
    
    //size_t tmp = 5 + matCount;
    size_t tmp = numNonMTL + mapCount;    
    textureFileNames = (char**)malloc(sizeof(char*)*tmp);

    for(size_t i = 0; i < tmp; i++)
        textureFileNames[i] = (char*)malloc(sizeof(char)*20);
    
    for(size_t i = 0; i < numNonMTL; i++)
        strcpy(textureFileNames[i], nonMTLTextures[i]);


    size_t tfIndex = 0;
    for(size_t i = 0; i < matCount; i++)
    {
        // Diffuse map
        if(matInfoList[i].map_Kd[0] != '\0')
        {
            bool duplicate = false;
            for(size_t j = 0; j < numNonMTL + tfIndex; j++)
                if(strcmp(matInfoList[i].map_Kd, textureFileNames[j]) == 0)
                    duplicate = true;

            if(!duplicate)
            {
                strcpy(textureFileNames[numNonMTL + tfIndex], matInfoList[i].map_Kd);
                tfIndex++;
            }
        }

        // Normal map
        if(matInfoList[i].map_bump[0] != '\0')
        {
            bool duplicate = false;
            for(size_t j = 0; j < numNonMTL + tfIndex; j++)
                if(strcmp(matInfoList[i].map_bump, textureFileNames[j]) == 0)
                    duplicate = true;

            if(!duplicate)
            {
                strcpy(textureFileNames[numNonMTL + tfIndex], matInfoList[i].map_bump);
                tfIndex++;
            }
        }

        // Alpha map
        if(matInfoList[i].map_d[0] != '\0')
        {
            bool duplicate = false;
            for(size_t j = 0; j < numNonMTL + tfIndex; j++)
                if(strcmp(matInfoList[i].map_d, textureFileNames[j]) == 0)
                    duplicate = true;

            if(!duplicate)
            {
                strcpy(textureFileNames[numNonMTL + tfIndex], matInfoList[i].map_d);
                tfIndex++;
            }            
        }

        // Specular map
        if(matInfoList[i].map_spec[0] != '\0')
        {
            bool duplicate = false;
            for(size_t j = 0; j < numNonMTL + tfIndex; j++)
                if(strcmp(matInfoList[i].map_spec, textureFileNames[j]) == 0)
                    duplicate = true;

            if(!duplicate)
            {
                strcpy(textureFileNames[numNonMTL + tfIndex], matInfoList[i].map_spec);
                tfIndex++;
            }            
        }                                    
    }
                                                                                  
    int numTextureFiles = tfIndex + numNonMTL;

    textureHandles = (GLuint*)malloc(sizeof(GLuint)*numTextureFiles);
    glGenTextures(numTextureFiles, textureHandles);

    glActiveTexture(GL_TEXTURE0);
    for(size_t i = 0; i < numTextureFiles; i++)
    {
        printf("%s\n", textureFileNames[i]);
        char buffer[100];
        buffer[0] = '\0';
        strcat(buffer, "../textures/");
        strcat(buffer, textureFileNames[i]);
        glBindTexture(GL_TEXTURE_2D, textureHandles[i]);
        loadAndSpecifyTexture(buffer);
        //glGenerateMipmap(GL_TEXTURE_2D);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    time(&endTime);
    double sec = difftime(endTime, startTime);
    printf("Texture loading took %f seconds.\n", sec);
    return numTextureFiles;
}

/*
  Find the texture handle specified by fileName in textureArray and send it to mat
  If the texture handle is not found, textureArray[i] is sent instead.
*/
void setMaterialTexture(Material *mat, const char* fileName, const char* uniformName,
                         char** textureFileNames, const GLuint* textureHandles, size_t numTextures)
{
    size_t i;
    for(i = 0; i < numTextures; i++)
        if(strcmp(fileName, textureFileNames[i]) == 0)
            break;

    if(i == numTextures)
        mat->sendUniformTexture(uniformName, textureHandles[1]);
    else
        mat->sendUniformTexture(uniformName, textureHandles[i]);    
}

void initMaterials(Material *materials[], const size_t arrayLen, int &numMat, const GLuint *textureHandles)
{
    // TODO: what if two materials have the same name?
    materials[numMat] = new Material(normalVertSrc, normalFragSrc, "Ship1Material");
    Vec3 color(1.0f, 1.0f, 1.0f);
    materials[numMat]->sendUniform3f("uColor", color);
    materials[numMat]->sendUniformTexture("diffuseMap", textureHandles[0]);
    materials[numMat]->sendUniformTexture("normalMap", textureHandles[2]);
    materials[numMat]->bindUniformBlock("UniformBlock", 0);
    ++numMat;

    materials[numMat] = new Material(normalVertSrc, normalFragSrc, "Ship2Material");
    materials[numMat]->sendUniform3f("uColor", color);
    materials[numMat]->sendUniformTexture("diffuseMap", textureHandles[3]);
    materials[numMat]->sendUniformTexture("normalMap", textureHandles[4]);
    materials[numMat]->bindUniformBlock("UniformBlock", 0);
    ++numMat;
    
    materials[numMat] = new Material(basicVertSrc, ADSFragSrc, "TeapotMaterial");
    materials[numMat]->sendUniform3f("uColor", color);
    materials[numMat]->sendUniformTexture("diffuseMap", textureHandles[1]);
    materials[numMat]->bindUniformBlock("UniformBlock", 0);
    ++numMat;
    
    
    materials[numMat] = new Material(basicVertSrc, basicFragSrc, "CubeMaterial");
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

void initMTLMaterials(MaterialInfo *matInfoList, const size_t matCount, Material *MTLMaterials[], int &numMTLMat,
                      char **textureFileNames, const GLuint *textureHandles, const int numTextures)
{
    enum ShaderFlag
    {
        DIFFUSE = 1,
        NORMAL = 2,
        SPECULAR = 4,
        ALPHA = 8
    };    

    // Initialize materials with data in matInfoList
    for(size_t i = 0; i < matCount; i++)
    {
        ShaderFlag sf = DIFFUSE;
        if(matInfoList[i].map_spec[0] != '\0')
            sf = static_cast<ShaderFlag>(static_cast<int>(sf) | static_cast<int>(SPECULAR));
        //if(matInfoList[i].map_bump[0] != '\0')
        if(strstr(matInfoList[i].map_bump, "ddn"))
            sf = static_cast<ShaderFlag>(static_cast<int>(sf) | static_cast<int>(NORMAL));
        if(matInfoList[i].map_d[0] != '\0')
            sf = static_cast<ShaderFlag>(static_cast<int>(sf) | static_cast<int>(ALPHA));

        switch(sf)
        {
        case DIFFUSE:
            //MTLMaterials[i] = new Material(basicVertSrc, OBJFragSrc, matInfoList[i].name);
            MTLMaterials[i] = new Material(shadowVertSrc, shOBJFragSrc, matInfoList[i].name);
            MTLMaterials[i]->setShadow(true);
            break;
        case DIFFUSE|NORMAL:
            MTLMaterials[i] = new Material(normalVertSrc, OBJNormalFragSrc, matInfoList[i].name);            
            break;
        case DIFFUSE|SPECULAR:
            //MTLMaterials[i] = new Material(basicVertSrc, OBJSpecFragSrc, matInfoList[i].name);
            MTLMaterials[i] = new Material(shadowVertSrc, shOBJSpecFragSrc, matInfoList[i].name);
            MTLMaterials[i]->setShadow(true);            
            break;
        case DIFFUSE|NORMAL|SPECULAR:
            MTLMaterials[i] = new Material(normalVertSrc, OBJNormalSpecFragSrc, matInfoList[i].name);            
            break;
        case DIFFUSE|ALPHA:
            // TODO: remove this line
            printf("alphafrag\n");
            //MTLMaterials[i] = new Material(basicVertSrc, OBJAlphaFragSrc, matInfoList[i].name);
            MTLMaterials[i] = new Material(shadowVertSrc, shOBJAlphaFragSrc, matInfoList[i].name);
            MTLMaterials[i]->setShadow(true);
            break;
        case DIFFUSE|NORMAL|ALPHA:
            MTLMaterials[i] = new Material(normalVertSrc, OBJNormalAlphaFragSrc, matInfoList[i].name);            
            break;
        case DIFFUSE|SPECULAR|ALPHA:
            //MTLMaterials[i] = new Material(basicVertSrc, OBJAlphaSpecFragSrc, matInfoList[i].name);
            MTLMaterials[i] = new Material(shadowVertSrc, shOBJAlphaSpecFragSrc, matInfoList[i].name);
            MTLMaterials[i]->setShadow(true);            
            break;
        case DIFFUSE|NORMAL|SPECULAR|ALPHA:
            MTLMaterials[i] = new Material(normalVertSrc, OBJNormalAlphaSpecFragSrc, matInfoList[i].name);            
            break;
        }
       
        //MTLMaterials[i]->sendUniform3f("Ka", matInfoList[i].Ka);
        MTLMaterials[i]->sendUniform3f("Kd", matInfoList[i].Kd);
        //MTLMaterials[i]->sendUniform3f("Ks", matInfoList[i].Ks);
        MTLMaterials[i]->sendUniform1f("Ns", matInfoList[i].Ns);
        MTLMaterials[i]->bindUniformBlock("UniformBlock", 0);

        if(matInfoList[i].name[0] == '\0')
            MTLMaterials[i]->sendUniformTexture("diffuseMap", textureHandles[1]);
        else
        {
            // Diffuse map
            setMaterialTexture(MTLMaterials[i], matInfoList[i].map_Kd, "diffuseMap", textureFileNames, textureHandles, numTextures);
            // Normal map
            if(sf & NORMAL)
                setMaterialTexture(MTLMaterials[i], matInfoList[i].map_bump, "normalMap", textureFileNames, textureHandles, numTextures);
            // Alpha map
            if(sf & ALPHA)
                setMaterialTexture(MTLMaterials[i], matInfoList[i].map_d, "alphaMap", textureFileNames, textureHandles, numTextures);
            // Specular map
            if(sf & SPECULAR)
                setMaterialTexture(MTLMaterials[i], matInfoList[i].map_spec, "specularMap", textureFileNames, textureHandles, numTextures);
        }
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
        if(strcmp(infoList[i].name, groupName) == 0)
            break;
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

// TODO: fix this so it's not using global variables

void initScene(TransformNode *rootNode, Geometries &geometries, Material *materials[], const int numMat, Material *MTLMaterials[],
               const int numMTLMat)
{
    RigTForm modelRbt;
    GeometryNode *gn;
    modelRbt = RigTForm(Vec3(-6.0f, 0.0f, 1.0f));
    Geometry *g = getSingleGeoFromArray(geometries.singleGeo, geometries.numSingleGeo, "Ship");
    Material *m = getMaterialFromArray(materials, numMat, "Ship1Material");
    if(g && m)
        gn = new GeometryNode(g, m, modelRbt, true);
    //rootNode->addChild(gn);
        

    modelRbt = RigTForm(Vec3(0.0f, 0.0f, -10.0f));
    g = getSingleGeoFromArray(geometries.singleGeo, geometries.numSingleGeo, "Ship2");
    m = getMaterialFromArray(materials, numMat, "Ship2Material");
    if(g && m)
        gn = new GeometryNode(g, m, modelRbt, true);
    //rootNode->addChild(gn);
    
    modelRbt = RigTForm(g_lightW);
    g = getSingleGeoFromArray(geometries.singleGeo, geometries.numSingleGeo, "cube");
    m = getMaterialFromArray(materials, numMat, "CubeMaterial");
    if(g && m)
    {
        gn = new GeometryNode(g, m, modelRbt, true);
        gn->setScaleFactor(Vec3(0.5f, 0.5f, 0.5f));
    }
    //rootNode->addChild(gn);

    modelRbt = RigTForm(Vec3(10.0f, 0.0f, 0.0f));
    g = getSingleGeoFromArray(geometries.singleGeo, geometries.numSingleGeo, "teapot");
    m = getMaterialFromArray(materials, numMat, "CubemapReflectionMat");
    if(g && m)
    {
        gn = new GeometryNode(g, m, modelRbt, true);
        gn->setScaleFactor(Vec3(1.0f/15.0f, 1.0f/15.0f, 1.0f/15.0f));
    }
    //rootNode->addChild(gn);

    MultiGeometryNode *mgn;
    //modelRbt = RigTForm(Vec3(0.0f, 0.0f, 0.0f), Quat::makeZRotation(-30.0f));
    modelRbt = RigTForm(Vec3(0.0f, 0.0f, 0.0f));
    // TODO: check if index is -1.
    int index = getGroupInfoFromArray(geometries.groupInfoList, geometries.numGroupInfo, "sponza");
    if(index != -1)
        mgn = new MultiGeometryNode(geometries.groupGeo, geometries.groupInfoList[index], MTLMaterials,
                                         numMTLMat, modelRbt, true);
    //rootNode->addChild(mgn);

    index = getGroupInfoFromArray(geometries.groupInfoList, geometries.numGroupInfo, "crysponza");
    if(index != -1)
    {
        mgn = new MultiGeometryNode(geometries.groupGeo, geometries.groupInfoList[index], MTLMaterials,
                                    numMTLMat, modelRbt, true);
        mgn->setScaleFactor(Vec3(1.0f/70.0f, 1.0f/70.0f, 1.0f/70.0f));
    }
    rootNode->addChild(mgn);
}

#endif
