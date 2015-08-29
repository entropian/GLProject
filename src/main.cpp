#include <GL/glew.h>

#if __GNUG__
#   include <GLFW/glfw3.h>
#else
#   include <GL/glfw3.h>
#endif

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <ctime>
#include <random>
/*
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
*/
#include "shaders.h"

#include "scenegraph.h"
#include "input.h"
#include "renderToBuffer.h"
#include "skybox.h"
#include "initresource.h"
#include "deferred.h"
#include "ssao.h"
#include "hdr.h"
#include "screenquad.h"
//#include "collision.h"

static float g_windowWidth = 1920.0f;
static float g_windowHeight = 1080.0f;

static RigTForm g_view;                // View transform
//static Vec3 g_lightE, g_lightW(15.0f, 25.0f, 2.0f);
Vec3 g_lightE, g_lightW(13.0f, 22.0f, 2.0f);

static Mat4 g_proj, g_ortho;
static Mat4 worldToLightSpaceMat;

// Materials
static const size_t MAX_MATERIALS = 200;

GLFWwindow* window;

static double g_framesPerSec = 60.0f;
static double g_distancePerSec = 3.0f;
static double g_timeBetweenFrames = 1.0 / g_framesPerSec;
static double g_distancePerFrame = g_distancePerSec / g_framesPerSec;

// Render-to-buffer
static RTB g_rtb;
static bool g_postProc = false;

// Skybox struct
Skybox g_skybox;
static bool g_drawSkybox = false;

// Depth map struct
DepthMap g_depthMap;
static Mat4 g_lightMat;
static bool g_depthMapStatus = true;

// Deferred shading struct
static DFStruct g_df;

// SSAO struct
static SSAOStruct g_ssaos;

// Uniform blocks
GLuint uniformBlock, uniformLightBlock;

static bool g_moveLight = false;
static bool g_shadow = true;
static double startTime;

InputHandler inputHandler;

static HDRStruct g_hdr;


void draw_scene(TransformNode *rootNode, Material *materials[], const int numMat, Material *MTLMaterials[], const int numMTLMat)
{
    RigTForm viewRbt = inputHandler.getViewTransform();
    // Draws scene from the perspective of the light
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
        visitor.visitNode(rootNode, g_depthMap.depthMapMaterial);
        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    glViewport(0, 0, (GLsizei)g_windowWidth, (GLsizei)g_windowHeight);
    
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate camera movement
    RigTForm trans(inputHandler.getMovementDir() * g_distancePerFrame);
    inputHandler.setViewTransform(trans * inputHandler.getViewTransform());

    // NOTE: since visitor already passes the view matrix, let it carry other often updated uniforms too,
    //RigTForm viewRbt = inputHandler.getViewTransform();
    if(g_moveLight)
    {
        double currentTime = glfwGetTime();
        g_lightW[0] = cos((currentTime - startTime) * 0.5) * 13.0f;
    }
    viewRbt = inputHandler.getViewTransform();

    // Update light position in view space
    g_lightE = viewRbt * g_lightW;
    glBindBuffer(GL_UNIFORM_BUFFER, uniformBlock);
    glBufferSubData(GL_UNIFORM_BUFFER, 64, 16, &(g_lightE[0]));

    // Update the matrix that transforms from view space to light space
    if(g_shadow)
    {
        //g_lightMat = Mat4::lookAt(g_lightW, Vec3(0.0f, 2.5f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
        g_lightMat = Mat4::lookAt(g_lightW, Vec3(0.0f, 2.5f, 0.0f), cross(g_lightW, Vec3(0.0f, 0.0f, 1.0f)));
        Mat4 lightSpaceMat = transpose(g_ortho * g_lightMat);
        glBufferSubData(GL_UNIFORM_BUFFER, 80, 64, &(lightSpaceMat[0]));
    }
    
    // Set inverse view matrix
    RigTForm invViewRbt = inv(viewRbt);
    Mat4 invViewMat = rigTFormToMat(invViewRbt);
    invViewMat = transpose(invViewMat);
    glBufferSubData(GL_UNIFORM_BUFFER, 144, 64, &(invViewMat[0]));
    
    // Draw skybox
    if(g_drawSkybox)
    {
        Mat4 viewRotationMat = rigTFormToMat(linFact(viewRbt));        
        drawSkybox(g_skybox, viewRotationMat);
    }

    // Geometry Pass
    glBindFramebuffer(GL_FRAMEBUFFER, g_df.gBuffer);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Traverse the scene graph and draw objects
    Visitor visitor(viewRbt);
    visitor.visitNode(inputHandler.getWorldNode());

    // SSAO Pass
    GLuint texArray[10], numTex = 0;
    texArray[numTex++] = g_df.gPositionDepth;
    texArray[numTex++] = g_df.gNormalSpec;
    texArray[numTex++] = g_ssaos.noiseTexture;
    drawScreenQuadMultiTex(g_ssaos.fbo, g_ssaos.firstPassProgram, g_ssaos.vao, texArray, numTex);

    // SSAO Blur Pass
    drawScreenQuad(g_ssaos.blurFbo, g_ssaos.blurPassProgram, g_ssaos.vao, g_ssaos.colorBuffer);

    // Display normal vectors
    //visitor.visitNode(inputHandler.getWorldNode(), g_showNormalMaterial);    

    // Light Pass with HDR
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);    
    numTex = 0;
    texArray[numTex++] = g_df.gPositionDepth;
    texArray[numTex++] = g_df.gNormalSpec;
    texArray[numTex++] = g_df.gDiffuse;
    texArray[numTex++] = g_depthMap.depthMap;
    texArray[numTex++] = g_ssaos.colorBufferBlur;        
    drawScreenQuadMultiTex(g_hdr.fbo, g_df.shaderProgram, g_df.vao, texArray, numTex);

    // Tone mapping pass
    drawScreenQuad(0, g_hdr.shaderProgram, g_hdr.vao, g_hdr.colorBuffer);
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
    switch(key)
    {
    case GLFW_KEY_1:
        if(action == GLFW_PRESS)
            g_df.shaderProgram = g_df.defaultShader;
        break;
    case GLFW_KEY_2:
        if(action == GLFW_PRESS)
            g_df.shaderProgram = g_df.showNormal;        
        break;
    case GLFW_KEY_3:
        if(action == GLFW_PRESS)
            g_df.shaderProgram = g_df.showDiffuse;         
        break;
    case GLFW_KEY_4:
        if(action == GLFW_PRESS)
            g_df.shaderProgram = g_df.showSSAO;
        break;
    case GLFW_KEY_5:
        if(action == GLFW_PRESS)
            g_df.shaderProgram = g_df.showSpecular;
        break;
    case GLFW_KEY_6:
        if(action == GLFW_PRESS)
            g_df.shaderProgram = g_df.shadowShader;
        break;
    case GLFW_KEY_M:
        if(action == GLFW_PRESS)
        {
            if(g_moveLight)
                g_moveLight = false;
            else
            {
                startTime = glfwGetTime();
                g_moveLight = true;
            }
        }
        break;                
    default:
        inputHandler.handleKey(window, key, scancode, action, mods);
    }
}


void initDepthMap(DepthMap* dm)
{
    glGenFramebuffers(1, &(dm->depthMapFBO));

    //dm->SHADOW_WIDTH = 2048;
    //dm->SHADOW_HEIGHT = 3074;
    dm->SHADOW_WIDTH = 2048;
    dm->SHADOW_HEIGHT = 3074;

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


void initUniformBlock()
{
    inputHandler.setViewTransform(RigTForm::lookAt(Vec3(7.0f, 7.0f, 1.0f), Vec3(0.0f, 5.0f, 0.0f), Vec3(0, 1, 0)));
    //inputHandler.setViewTransform(RigTForm::lookAt(g_lightW, Vec3(0.0f, 2.5f, 0.0f), Vec3(0.0f, 1.0, 0.0f)));
    g_lightMat = Mat4::lookAt(g_lightW, Vec3(0.0f, 2.5f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));    

    // Initialize Matrices uniform block
    /* Block layout
      layout (std140) uniform UniformBlock
      {
      //                     Base alignment    Aligned offset
      mat4 projMat;       // 16 (x4)           0
      vec3 light1;        // 16                64
      mat4 lightSpaceMat  // 16 (x4)           80
      mat4 invViewMat     // 16 (x4)           144
    };
    16 + 64 + 64 = 144
    16 + 64 + 64 + 64 = 210
    */
    glGenBuffers(1, &uniformBlock);
    glBindBuffer(GL_UNIFORM_BUFFER, uniformBlock);
    glBufferData(GL_UNIFORM_BUFFER, 210, NULL, GL_STATIC_DRAW);
    // Projection matrix
    Mat4 proj = transpose(g_proj);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Mat4), &(proj[0]));
    // Light space matrix
    /*
    Mat4 tmp = Mat4::makeOrtho(-9.0f, 8.0f, -20.0f, 27.0f, 7.0f, 35.0f);
    Mat4 ortho = tmp;
    //Mat4 lightSpaceMat = transpose(g_proj * g_lightMat);
    Mat4 lightSpaceMat = transpose(ortho * g_lightMat);
    glBufferSubData(GL_UNIFORM_BUFFER, 80, 64, &(lightSpaceMat[0]));
    */

    // light pos in world space
    //glBufferSubData(GL_UNIFORM_BUFFER, 64, 16, &(g_lightW[0]));

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBlock);
}


/*
void deleteScenegraph(TransformNode *tn)
{
    for(int i = 0; i < tn->getChildrenCount(); i++)
        deleteScenegraph(static_cast<TransformNode*>(tn->getChild(i)));
    delete tn;
}
*/
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

    int a;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &a);
    printf("Max texture image units = %d\n", a);    

    // ----------------------------- RESOURCES ----------------------------- //
    inputHandler.initialize();
    Geometries geometries;

    // Geometries    
    initGeometries(geometries);


    for(int i = 0; i < geometries.numSingleGeo; i++)
    {
        AABB aabb = geometries.singleGeo[i]->aabb;
        Vec3 min = aabb.getMin();
        Vec3 max = aabb.getMax();        
        printf("i = %d\n", i);
        printf("min = %f, %f, %f\n", min[0], min[1], min[2]);
        printf("max = %f, %f, %f\n", max[0], max[1], max[2]);        
    }

    
    MaterialInfo matInfoList[MAX_MATERIALS];
    const size_t numMTLFiles = 2;
    char MTLFileNames[numMTLFiles][20] = {"sponza.mtl", "crysponza.mtl"};
    
    size_t MTLMatCount = loadMTLFiles(matInfoList, MAX_MATERIALS, MTLFileNames, numMTLFiles);
    
    char *nonMTLTextures[5] = {"Ship_Diffuse.png", "default.png", "Ship_Normal.png",
                                "Ship2_Diffuse.png", "Ship2_Normal.png"};
    int numNonMTL = 5;
    GLuint *textureHandles;
    int numTextures;
    char **textureFileNames;

    numTextures = initTextures(matInfoList, MTLMatCount, textureFileNames, nonMTLTextures, numNonMTL, textureHandles);

    /*
    // TODO: figure out why I can't have it as g_proj = Mat4::makeOrtho()
    //Mat4 ortho2 = Mat4::makeOrtho(-10.0f, 10.0f, -10.0f, 10.0f, .05f, 25.0f);
    Mat4 ortho2 = Mat4::makeOrtho(-10.0f, 10.0f, -20.0f, 20.0f, 1.0f, 60.0f);
    g_proj = ortho2;
    */
    g_proj = Mat4::makePerspective(60.0f, g_windowWidth/g_windowHeight, 0.1f, 50.0f);

    g_lightMat = Mat4::lookAt(g_lightW, Vec3(0.0f, 2.5f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));    
    initSkybox(g_skybox);

    initUniformBlock();
 
    Material *materials[MAX_MATERIALS];
    int numMat = 0;
    initMaterials(materials, MAX_MATERIALS, numMat, textureHandles);
    
    Material *MTLMaterials[MAX_MATERIALS];
    int numMTLMat = 0;
    initMTLMaterials(matInfoList, MTLMatCount, MTLMaterials, numMTLMat, textureFileNames, textureHandles, numTextures);

    TransformNode *worldNode = new TransformNode();
    inputHandler.setWorldNode(worldNode);

    const int MAX_NUM_GEO_NODES = 50;
    BaseGeometryNode *baseGeoNodes[MAX_NUM_GEO_NODES];
    int numGeoNodes = 0;
    initScene(worldNode, geometries, materials, numMat, MTLMaterials, numMTLMat, baseGeoNodes,
              MAX_NUM_GEO_NODES, numGeoNodes);
    initRenderToBuffer(g_rtb, (int)g_windowWidth, (int)g_windowHeight);
    initDepthMap(&g_depthMap);
    initDeferredRender(g_df, (int)g_windowWidth, (int)g_windowHeight);
    initSSAO(g_ssaos, (int)g_windowWidth, (int)g_windowHeight, 64, 1.0);
    initHDR(g_hdr, (int)g_windowWidth, (int)g_windowHeight);

    for(int i = 0; i < numMTLMat; i++)        
        MTLMaterials[i]->sendUniformTexture("shadowMap", g_depthMap.depthMap);

    Mat4 tmp = Mat4::makeOrtho(-9.0f, 8.0f, -27.0f, 27.0f, 5.0f, 35.0f);
    g_ortho = tmp;    

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;    

    // ---------------------------- RENDERING ------------------------------ //
    
    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);    
    draw_scene(worldNode, materials, numMat, MTLMaterials, numMTLMat);
    double currentTime, timeLastRender = 0;
    while(!glfwWindowShouldClose(window))
    {
        if(inputHandler.getInputMode() != PICKING_MODE)
            currentTime = glfwGetTime();
            if((currentTime - timeLastRender) >= g_timeBetweenFrames)
            {
                glfwSwapBuffers(window);
                timeLastRender = currentTime;
                draw_scene(worldNode, materials, numMat, MTLMaterials, numMTLMat);
            }

        // Poll window events
        glfwPollEvents();
    }

    // ---------------------------- Clearing ------------------------------ //

    // Delete allocated resources
    glDeleteTextures(numTextures, textureHandles);
    for(int i = 0; i < geometries.numSingleGeo; i++)
        delete geometries.singleGeo[i];

    for(int i = 0; i < geometries.numGroupGeo; i++)
        delete geometries.groupGeo[i];
    
    for(int i = 0; i < numMat; i++)
        delete materials[i];

    for(int i = 0; i < numMTLMat; i++)
        delete MTLMaterials[i];
    delete g_depthMap.depthMapMaterial;
    deleteSSAO(g_ssaos);
    deleteDeferredStruct(g_df);
    deleteSkybox(g_skybox);
    deleteHDRStruct(g_hdr);
    
    //deleteScenegraph(worldNode);    
    // ---------------------------- TERMINATE ----------------------------- //

    // Terminate GLFW
    glfwTerminate();

    return 0;
}
