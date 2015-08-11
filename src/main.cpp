#include <GL/glew.h>

#if __GNUG__
#   include <GLFW/glfw3.h>
#   include "SOIL/SOIL.h"
#else
#   include <GL/glfw3.h>
#   include "SOIL.h"
#endif

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <ctime>
/*
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
*/
#include "fileIO.h"
#include "shaders.h"

//#include "scenegraph.h"
#include "input.h"
#include "mesh.h"
#include "renderToBuffer.h"
#include "skybox.h"


static float g_windowWidth = 1280.0f;
static float g_windowHeight = 720.0f;


static RigTForm g_view;                // View transform
static Vec3 g_lightE, g_lightW(15.0f, 25.0f, 2.0f);
static Mat4 g_proj;

// Geometries
static Geometry *g_cube, *g_floor, *g_wall, *g_ship1, *g_ship2, *g_terrain, *g_teapot, *g_sponza, *g_crysponza;
static const size_t MAX_GEOMETRY_GROUPS = 1000;
static Geometry *g_geometryGroups[MAX_GEOMETRY_GROUPS];
static size_t g_groupSize = 0;

static GeoGroupInfo g_groupInfoList[MAX_GEOMETRY_GROUPS];
static int g_groupInfoSize = 0;

// Scenegraph nodes
static TransformNode *g_worldNode;  // Root node
static GeometryNode *g_terrainNode, *g_cubeArray[4], *g_cubeNode, *g_teapotNode, *g_ship2Node;
static MultiGeometryNode *g_sponzaNode;
//static GeometryNode *g_crysponzaNode;
static MultiGeometryNode *g_crysponzaNode;


// Materials
static Material *g_shipMaterial1, *g_shipMaterial2, *g_pickMaterial, *g_cubeMaterial, *g_teapotMaterial;
static Material *g_cubemapReflectionMat, *g_showNormalMaterial;
static const size_t MAX_MATERIALS = 200;
static Material *g_materials[MAX_MATERIALS];
static int g_numMat = 0;

// Texture handles
static GLuint *textures;
static char **g_textureFileNames;
static size_t g_numTextures;

GLFWwindow* window;

static double g_framesPerSec = 60.0f;
static double g_distancePerSec = 3.0f;
static double g_timeBetweenFrames = 1.0 / g_framesPerSec;
static double g_distancePerFrame = g_distancePerSec / g_framesPerSec;

// Render-to-buffer
static RTB g_rtb;
static bool g_renderToBuffer = true;

// Skybox struct

static Skybox g_skybox;
static bool g_drawSkybox = true;

// Depth map struct
struct DepthMap{
    GLuint depthMapFBO;
    GLuint depthMap;
    GLsizei SHADOW_WIDTH, SHADOW_HEIGHT;
    Material *depthMapMaterial;
};
static DepthMap g_depthMap;
static Mat4 g_lightMat;
static bool g_depthMapStatus = false;




// Uniform blocks
GLuint uniformBlock, uniformLightBlock;

InputHandler inputHandler;

void draw_scene()
{
    RigTForm viewRbt = inputHandler.getViewTransform();        
    if(g_depthMapStatus)
    {

        glViewport(0, 0, g_depthMap.SHADOW_WIDTH, g_depthMap.SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, g_depthMap.depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        //Mat4 viewMat = rigTFormToMat(inputHandler.getViewTransform());
        //viewMat = g_proj * viewMat;
        //g_depthMap.depthMapMaterial->sendUniformMat4("uLightSpaceMat", viewMat);
        glCullFace(GL_FRONT);
        Visitor visitor(viewRbt);
        visitor.visitNode(inputHandler.getWorldNode(), g_depthMap.depthMapMaterial);
        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        for(int i = 0; i < g_numMat; i++)
        {
            g_materials[i]->sendUniformTexture("shadowMap", g_depthMap.depthMap);
        }
    }
    
    glViewport(0, 0, (GLsizei)g_windowWidth, (GLsizei)g_windowHeight);
    
    if(g_renderToBuffer)
        glBindFramebuffer(GL_FRAMEBUFFER, g_rtb.framebuffer);
    
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate camera movement
    RigTForm trans(inputHandler.getMovementDir() * g_distancePerFrame);
    inputHandler.setViewTransform(trans * inputHandler.getViewTransform());

    // NOTE: since visitor already passes the view matrix, let it carry other often updated uniforms too,
    // like g_lightE
    // Update some uniforms
    //RigTForm viewRbt = inputHandler.getViewTransform();
    //double currentTime = glfwGetTime();
    //g_lightW[0] = g_lightW[0] + sin(currentTime) * 1;
    g_lightE = viewRbt * g_lightW;

    glBindBuffer(GL_UNIFORM_BUFFER, uniformBlock);
    glBufferSubData(GL_UNIFORM_BUFFER, 64, 16, &(g_lightE[0]));    

    
    // Draw skybox
    if(g_drawSkybox)
    {
        Mat4 viewRotationMat = rigTFormToMat(linFact(viewRbt));        
        drawSkybox(g_skybox, viewRotationMat);
    }

    // Draw scene
    Visitor visitor(viewRbt);
    visitor.visitNode(inputHandler.getWorldNode());

    // Display normal vectors
    //visitor.visitNode(inputHandler.getWorldNode(), g_showNormalMaterial);
    

    // Draws the image in the framebuffer onto the screen
    if(g_renderToBuffer)
        drawBufferToScreen(g_rtb);

    //glfwSwapBuffers(window);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    inputHandler.handleMouseButton(window, button, action, mods);
}

void cursorPosCallback(GLFWwindow* window, double x, double y)
{
    inputHandler.handleCursor(window, x, y);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    inputHandler.handleKey(window, key, scancode, action, mods);
}


void initGeometries()
{
    Mesh ship1Mesh;
    ship1Mesh.loadOBJFile("Ship.obj");
    ship1Mesh.computeVertexBasis();
    g_ship1 = ship1Mesh.produceGeometry(PNXTBD);

    Mesh ship2Mesh;
    ship2Mesh.loadOBJFile("Ship2.obj");
    ship2Mesh.computeVertexBasis();
    g_ship2 = ship2Mesh.produceGeometry(PNXTBD);

    Mesh cubeMesh;
    cubeMesh.loadOBJFile("cube.obj");
    g_cube = cubeMesh.produceGeometry(PNX);

    Mesh teapotMesh;
    teapotMesh.loadOBJFile("teapot.obj");
    teapotMesh.computeVertexNormals();
    g_teapot = teapotMesh.produceGeometry(PNX);

    Mesh sponzaMesh;
    sponzaMesh.loadOBJFile("sponza.obj");
    sponzaMesh.computeVertexNormals();
    getGeoList(sponzaMesh, g_geometryGroups, g_groupInfoList, MAX_GEOMETRY_GROUPS, g_groupSize, g_groupInfoSize, PNX);    
    //getGeoList(sponzaMesh, g_geometryGroups, g_groupInfoList, MAX_GEOMETRY_GROUPS, g_groupSize, g_groupInfoSize, PNXTBD);

    
    Mesh crysponzaMesh;
    crysponzaMesh.loadOBJFile("crysponza.obj");
    crysponzaMesh.computeVertexBasis();
    //getGeoList(crysponzaMesh, g_geometryGroups, g_groupInfoList, MAX_GEOMETRY_GROUPS, g_groupSize, g_groupInfoSize, PNX);
    getGeoList(crysponzaMesh, g_geometryGroups, g_groupInfoList, MAX_GEOMETRY_GROUPS, g_groupSize, g_groupInfoSize, PNXTBD);

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SOIL_free_image_data(image);
}


void initDepthMap(DepthMap* dm)
{
    glGenFramebuffers(1, &(dm->depthMapFBO));

    dm->SHADOW_WIDTH = 2048;
    dm->SHADOW_HEIGHT = 3072;

    glGenTextures(1, &(dm->depthMap));
    glBindTexture(GL_TEXTURE_2D, dm->depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                 dm->SHADOW_WIDTH, dm->SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);      

    glBindFramebuffer(GL_FRAMEBUFFER, dm->depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dm->depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    dm->depthMapMaterial = new Material(depthMapVertSrc, depthMapFragSrc, "DepthMapMaterial");
    dm->depthMapMaterial->setDepthMap(true);
}


// Initializes textures that are used in materials
size_t initTextures(MaterialInfo matInfoList[], const size_t matCount, char **&textureFileNames)
{
    time_t startTime, endTime;
    time(&startTime);
    char *nonMTLTextures[5] = {"Ship_Diffuse.png", "default.png", "Ship_Normal.png",
                                "Ship2_Diffuse.png", "Ship2_Normal.png"};

    size_t numNonMTL = 5;
    
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
                                                                                  
    size_t numTextureFiles = tfIndex + numNonMTL;

    textures = (GLuint*)malloc(sizeof(GLuint)*numTextureFiles);
    glGenTextures(numTextureFiles, textures);

    glActiveTexture(GL_TEXTURE0);
    for(size_t i = 0; i < numTextureFiles; i++)
    {
        printf("%s\n", textureFileNames[i]);
        char buffer[100];
        buffer[0] = '\0';
        strcat(buffer, "../textures/");
        strcat(buffer, textureFileNames[i]);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
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
                         char** textureFileNames, GLuint* textureHandles, size_t numTextures)
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

void initUniformBlock()
{
    inputHandler.setViewTransform(RigTForm::lookAt(Vec3(10.0f, 10.0f, 8.0f), Vec3(4.0f, 0.0f, 0.0f), Vec3(0, 1, 0)));
    //inputHandler.setViewTransform(RigTForm::lookAt(g_lightW, Vec3(0.0f, 2.5f, 0.0f), Vec3(0.0f, 1.0, 0.0f)));
    //g_lightMat = Mat4::lookAt(g_lightW, Vec3(0.0f, 2.5f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));    

    // Initialize Matrices uniform block
    /* Block layout
      layout (std140) uniform UniformBlock
      {
      //                     Base alignment    Aligned offset
      mat4 projMat;       // 16 (x4)           0
      vec3 light1;        // 16                64
      mat4 lightSpaceMat // 16 (x4)           80
    };
    16 + 64 + 64 = 144
    */
    glGenBuffers(1, &uniformBlock);
    glBindBuffer(GL_UNIFORM_BUFFER, uniformBlock);
    glBufferData(GL_UNIFORM_BUFFER, 144, NULL, GL_STATIC_DRAW);
    // Projection matrix
    Mat4 proj = transpose(g_proj);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Mat4), &(proj[0]));
    // Light space matrix
    Mat4 tmp = Mat4::makeOrtho(-10.0f, 10.0f, -20.0f, 20.0f, 1.0f, 60.0f);
    Mat4 ortho = tmp;
    //Mat4 lightSpaceMat = transpose(g_proj * g_lightMat);
    Mat4 lightSpaceMat = transpose(ortho * g_lightMat);
    glBufferSubData(GL_UNIFORM_BUFFER, 80, 64, &(lightSpaceMat[0]));
    
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBlock);
}

void initMaterials()
{    
    // TODO: what if two materials have the same name?
    g_shipMaterial1 = new Material(normalVertSrc, normalFragSrc, "ShipMaterial1");
    //g_shipMaterial1 = new Material(normalVertSrc, normalFragSrc, exampleGeoSrc, "ShipMaterial1");    
    Vec3 color(1.0f, 1.0f, 1.0f);
    g_shipMaterial1->sendUniform3f("uColor", color);
    g_shipMaterial1->sendUniformTexture("diffuseMap", textures[0]);
    g_shipMaterial1->sendUniformTexture("normalMap", textures[2]);
    g_shipMaterial1->bindUniformBlock("UniformBlock", 0);

    
    g_shipMaterial2 = new Material(normalVertSrc, normalFragSrc, "ShipMaterial2");
    g_shipMaterial2->sendUniform3f("uColor", color);
    g_shipMaterial2->sendUniformTexture("diffuseMap", textures[3]);
    g_shipMaterial2->sendUniformTexture("normalMap", textures[4]);
    g_shipMaterial2->bindUniformBlock("UniformBlock", 0);    

    g_teapotMaterial = new Material(basicVertSrc, ADSFragSrc, "TeapotMaterial");
    g_teapotMaterial->sendUniform3f("uColor", color);
    g_teapotMaterial->sendUniformTexture("diffuseMap", textures[1]);
    g_teapotMaterial->bindUniformBlock("UniformBlock", 0);    

    g_cubeMaterial = new Material(basicVertSrc, basicFragSrc, "CubeMaterial");
    //g_cubeMaterial = new Material(basicVertSrc, diffuseFragSrc, exampleGeoSrc, "CubeMaterial");    
    //g_cubeMaterial->sendUniform3f("uColor", Vec3(1.0f, 1.0f, 1.0f));
    g_cubeMaterial->bindUniformBlock("UniformBlock", 0);    

    g_cubemapReflectionMat = new Material(cubemapReflectionVertSrc, cubemapReflectionFragSrc, "CubemapReflection");
    // TODO: change the source of the cubemap to something else that isn't the skybox struct
    g_cubemapReflectionMat->sendUniformCubemap("uCubemap", g_skybox.cubemap);
    g_cubemapReflectionMat->bindUniformBlock("UniformBlock", 0);

    g_showNormalMaterial = new Material(showNormalVertSrc, basicFragSrc, showNormalGeoSrc, "showNormalMaterial");
    g_showNormalMaterial->bindUniformBlock("UniformBlock", 0);
}


void initMTLMaterials(MaterialInfo *matInfoList, const size_t matCount, Material *materials[], int &numMat,
                      char **textureFileNames, const GLuint *textureHandles, const size_t numTextures)
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
        if(matInfoList[i].map_bump[0] != '\0')
            sf = static_cast<ShaderFlag>(static_cast<int>(sf) | static_cast<int>(NORMAL));
        if(matInfoList[i].map_d[0] != '\0')
            sf = static_cast<ShaderFlag>(static_cast<int>(sf) | static_cast<int>(ALPHA));

        switch(sf)
        {
        case DIFFUSE:
            materials[i] = new Material(basicVertSrc, OBJFragSrc, matInfoList[i].name);
            break;
        case DIFFUSE|NORMAL:
            materials[i] = new Material(normalVertSrc, OBJNormalFragSrc, matInfoList[i].name);            
            break;
        case DIFFUSE|SPECULAR:
            materials[i] = new Material(basicVertSrc, OBJSpecFragSrc, matInfoList[i].name);            
            break;
        case DIFFUSE|NORMAL|SPECULAR:
            materials[i] = new Material(normalVertSrc, OBJNormalSpecFragSrc, matInfoList[i].name);            
            break;
        case DIFFUSE|ALPHA:
            materials[i] = new Material(basicVertSrc, OBJAlphaFragSrc, matInfoList[i].name);            
            break;
        case DIFFUSE|NORMAL|ALPHA:
            materials[i] = new Material(normalVertSrc, OBJNormalAlphaFragSrc, matInfoList[i].name);            
            break;
        case DIFFUSE|SPECULAR|ALPHA:
            materials[i] = new Material(basicVertSrc, OBJAlphaSpecFragSrc, matInfoList[i].name);            
            break;
        case DIFFUSE|NORMAL|SPECULAR|ALPHA:
            materials[i] = new Material(normalVertSrc, OBJNormalAlphaSpecFragSrc, matInfoList[i].name);            
            break;
        }
       
        //materials[i]->sendUniform3f("Ka", matInfoList[i].Ka);
        materials[i]->sendUniform3f("Kd", matInfoList[i].Kd);
        //materials[i]->sendUniform3f("Ks", matInfoList[i].Ks);
        materials[i]->sendUniform1f("Ns", matInfoList[i].Ns);
        materials[i]->bindUniformBlock("UniformBlock", 0);

        if(matInfoList[i].name[0] == '\0')
            materials[i]->sendUniformTexture("diffuseMap", textures[1]);
        else
        {
            // Diffuse map
            setMaterialTexture(materials[i], matInfoList[i].map_Kd, "diffuseMap", textureFileNames, textures, numTextures);
            // Normal map
            if(sf & NORMAL)
                setMaterialTexture(materials[i], matInfoList[i].map_bump, "normalMap", textureFileNames, textures, numTextures);
            // Alpha map
            if(sf & ALPHA)
                setMaterialTexture(materials[i], matInfoList[i].map_d, "alphaMap", textureFileNames, textures, numTextures);
            // Specular map
            if(sf & SPECULAR)
                setMaterialTexture(materials[i], matInfoList[i].map_spec, "specularMap", textureFileNames, textures, numTextures);
        }
    }
    numMat += matCount;
}

void initScene()
{
    g_worldNode = new TransformNode();
    inputHandler.setWorldNode(g_worldNode);

    RigTForm modelRbt;
    modelRbt = RigTForm(Vec3(-6.0f, 0.0f, 1.0f));
    g_terrainNode = new GeometryNode(g_ship1, g_shipMaterial1, modelRbt, true);

    modelRbt = RigTForm(Vec3(0.0f, 0.0f, -10.0f));
    g_ship2Node = new GeometryNode(g_ship2, g_shipMaterial2, modelRbt, true);
    
    modelRbt = RigTForm(g_lightW);
    g_cubeNode = new GeometryNode(g_cube, g_cubeMaterial, modelRbt, true);
    g_cubeNode->setScaleFactor(Vec3(0.5f, 0.5f, 0.5f));

    modelRbt = RigTForm(Vec3(10.0f, 0.0f, 0.0f));
    //g_teapotNode = new GeometryNode(g_teapot, g_teapotMaterial, modelRbt, true);
    g_teapotNode = new GeometryNode(g_teapot, g_cubemapReflectionMat, modelRbt, true);
    g_teapotNode->setScaleFactor(Vec3(1.0f/15.0f, 1.0f/15.0f, 1.0f/15.0f));

    //modelRbt = RigTForm(Vec3(0.0f, 0.0f, 0.0f), Quat::makeZRotation(-30.0f));
    modelRbt = RigTForm(Vec3(0.0f, 0.0f, 0.0f));
    g_sponzaNode = new MultiGeometryNode(g_geometryGroups, g_groupInfoList[0], g_materials, g_numMat, modelRbt, true);

    g_crysponzaNode = new MultiGeometryNode(g_geometryGroups, g_groupInfoList[1], g_materials, g_numMat, modelRbt, true);
    g_crysponzaNode->setScaleFactor(Vec3(1.0f/55.0f, 1.0f/55.0f, 1.0f/55.0f));



    g_worldNode->addChild(g_terrainNode);
    g_worldNode->addChild(g_ship2Node);
    g_worldNode->addChild(g_cubeNode);
    g_worldNode->addChild(g_teapotNode);
    //g_worldNode->addChild(g_sponzaNode);
    //g_worldNode->addChild(g_crysponzaNode);
}



int main()
{
    // -------------------------------- INIT ------------------------------- //


    
    // Init GLFW
    if (glfwInit() != GL_TRUE) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    
    // Create a rendering window with OpenGL 3.3 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);    

    //window = glfwCreateWindow(800, 600, "OpenGL", NULL, NULL);
    window = glfwCreateWindow((int)g_windowWidth, (int)g_windowHeight, "OpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);

    // Init GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetKeyCallback(window, keyCallback);
    glFrontFace(GL_CCW);

    int a;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &a);
    printf("Max texture image units = %d\n", a);    

    // ----------------------------- RESOURCES ----------------------------- //

    inputHandler.initialize();    
    initGeometries();        
    MaterialInfo matInfoList[MAX_MATERIALS];

    const size_t numMTLFiles = 2;
    char MTLFileNames[numMTLFiles][20] = {"sponza.mtl", "crysponza.mtl"};
    
    size_t matCount = loadMTLFiles(matInfoList, MAX_MATERIALS, MTLFileNames, numMTLFiles);
    g_numTextures = initTextures(matInfoList, matCount, g_textureFileNames);

    /*
    // TODO: figure out why I can't have it as g_proj = Mat4::makeOrtho()
    Mat4 ortho2 = Mat4::makeOrtho(-10.0f, 10.0f, -10.0f, 10.0f, .05f, 25.0f);
    g_proj = ortho2;
    */
    g_proj = Mat4::makePerspective(60.0f, g_windowWidth/g_windowHeight, 0.1f, 50.0f);
    //Mat4 tmp = Mat4::makeOrtho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 35.0f);
    //g_proj = tmp;

    g_lightMat = Mat4::lookAt(g_lightW, Vec3(0.0f, 2.5f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));    
    initSkybox(g_skybox);

    initUniformBlock();
    initMaterials();
    initMTLMaterials(matInfoList, matCount, g_materials, g_numMat, g_textureFileNames, textures, g_numTextures);
    initScene();
    initRenderToBuffer(g_rtb, (int)g_windowWidth, (int)g_windowHeight);
    initDepthMap(&g_depthMap);

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;    

    // ---------------------------- RENDERING ------------------------------ //
    
    glEnable(GL_DEPTH_TEST);
    draw_scene();
    double currentTime, timeLastRender = 0;
    while(!glfwWindowShouldClose(window))
    {
        if(inputHandler.getInputMode() != PICKING_MODE)
        {
            currentTime = glfwGetTime();
            if((currentTime - timeLastRender) >= g_timeBetweenFrames)
            {
                glfwSwapBuffers(window);
                timeLastRender = currentTime;
                draw_scene();
            }
        }

        // Poll window events
        glfwPollEvents();

    }

    // ---------------------------- Clearing ------------------------------ //

    // Delete allocated resources


    glDeleteTextures(g_numTextures, textures);
    /*
    glDeleteBuffers(1, &(g_cube->vbo));
    glDeleteBuffers(1, &(g_ship1->vbo));
    glDeleteBuffers(1, &(g_ship2->vbo));
    glDeleteBuffers(1, &(g_teapot->vbo));
    glDeleteVertexArrays(1, &(g_cube->vao));
    glDeleteVertexArrays(1, &(g_ship1->vao));
    glDeleteVertexArrays(1, &(g_ship2->vao));
    glDeleteVertexArrays(1, &(g_teapot->vao));
    */
    delete g_cube;
    delete g_ship1;
    delete g_ship2;
    delete g_teapot;

    delete g_shipMaterial1;
    delete g_shipMaterial2;
    delete g_cubeMaterial;
    delete g_cubemapReflectionMat;
    delete g_showNormalMaterial;
    delete g_depthMap.depthMapMaterial;
    delete g_teapotMaterial;

    for(size_t i = 0; i < g_groupInfoSize; i++)
        delete g_geometryGroups[i];
    for(size_t i = 0; i < g_numMat; i++)
        delete g_materials[i];
    // ---------------------------- TERMINATE ----------------------------- //

    // Terminate GLFW
    glfwTerminate();

    return 0;
}
