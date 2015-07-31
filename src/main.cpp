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

#include "fileIO.h"
#include "shaders.h"

//#include "scenegraph.h"
#include "input.h"
#include "mesh.h"

#define GLSL(src) "#version 150 core\n" #src

static float g_windowWidth = 1280.0f;
static float g_windowHeight = 720.0f;

// some of the shader uniforms
static RigTForm g_view;
static Vec3 g_lightE, g_lightW(0.0f, 9.5f, 0.0f);
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
static GeometryNode *g_crysponzaNode;


// Materials
static Material *g_shipMaterial1, *g_shipMaterial2, *g_pickMaterial, *g_cubeMaterial, *g_teapotMaterial;
static Material *g_cubemapReflectionMat;
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

// struct for Render-to-buffer 
struct RTB{
    GLuint framebuffer, texColorBuffer, rbo;
    GLuint vao, vbo;
    GLuint shaderProgram;
    GLuint texture;
};
static RTB g_rtb;
static bool g_renderToBuffer = true;

// Skybox struct
struct Skybox{
    GLuint cubemap, cubemapUniformHandle, viewMatHandle;
    GLuint vao, vbo, shaderProgram;    
};
static Skybox g_skybox;
static bool g_drawSkybox = true;


InputHandler inputHandler;

void draw_scene()
{
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
    g_lightE = inputHandler.getViewTransform() * g_lightW;
    g_shipMaterial1->sendUniform3f("uLight", g_lightE);
    g_shipMaterial2->sendUniform3f("uLight", g_lightE);    
    g_cubeMaterial->sendUniform3f("uLight", g_lightE);
    g_teapotMaterial->sendUniform3f("uLight", g_lightE);


    for(int i = 0; i < g_numMat; i++)
        g_materials[i]->sendUniform3f("uLight", g_lightE);

    RigTForm viewRbt = inputHandler.getViewTransform();
    Mat4 invViewMat = rigTFormToMat(inv(viewRbt));
    /*
    for(int i = 0; i < 16; i++)
    {
        if(i == 0)
            printf("Matrix\n");
        printf("%f ", invViewMat[i]);
        if(((i + 1) % 4) == 0)
            printf("\n");
    }
    */
    g_cubemapReflectionMat->sendUniformMat4("uInvViewMat", invViewMat);
    
    // Draw skybox
    if(g_drawSkybox)
    {
        glDepthMask(GL_FALSE);
        glUseProgram(g_skybox.shaderProgram);
        Mat4 viewRotationMat = rigTFormToMat(linFact(viewRbt));
        glUniformMatrix4fv(g_skybox.viewMatHandle, 1, GL_TRUE, &(viewRotationMat[0]));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, g_skybox.cubemap);
        glUniform1i(g_skybox.cubemapUniformHandle, 0);
        glBindVertexArray(g_skybox.vao);
        glBindBuffer(GL_ARRAY_BUFFER, g_skybox.vbo);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);        
        glDepthMask(GL_TRUE);  
    }



    // Draw scene
    Visitor visitor(inputHandler.getViewTransform());
    visitor.visitNode(inputHandler.getWorldNode());
    

    // Draws the image in the framebuffer onto the screen
    if(g_renderToBuffer)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);                
        glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glUseProgram(g_rtb.shaderProgram);
        glBindVertexArray(g_rtb.vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_rtb.texColorBuffer);
        glUniform1i(g_rtb.texture, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);
    }
    
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
    g_ship1 = ship1Mesh.produceGeometryPNXTBD();

    Mesh ship2Mesh;
    ship2Mesh.loadOBJFile("Ship2.obj");
    ship2Mesh.computeVertexBasis();
    g_ship2 = ship2Mesh.produceGeometryPNXTBD();

    Mesh cubeMesh;
    cubeMesh.loadOBJFile("cube.obj");
    g_cube = cubeMesh.produceGeometryPNX();

    Mesh teapotMesh;
    teapotMesh.loadOBJFile("teapot.obj");
    teapotMesh.computeVertexNormals();
    g_teapot = teapotMesh.produceGeometryPNX();

    Mesh sponzaMesh;
    sponzaMesh.loadOBJFile("sponza.obj");
    sponzaMesh.computeVertexNormals();
    //sponzaMesh.computeVertexBasis();
    getGeoList(sponzaMesh, g_geometryGroups, g_groupInfoList, MAX_GEOMETRY_GROUPS, g_groupSize, g_groupInfoSize, PNX);

    Mesh crysponzaMesh;
    crysponzaMesh.loadOBJFile("crysponza.obj");
    g_crysponza = crysponzaMesh.produceGeometryPNX();
    //getGeoList(crysponzaMesh, g_geometryGroups, g_groupInfoList, MAX_GEOMETRY_GROUPS, g_groupSize, g_groupInfoSize, PNX);
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

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SOIL_free_image_data(image);
}

GLuint loadCubemap(const char *faces[])
{
    GLuint textureID;
    glGenTextures(1,&textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);


    int width, height;
    unsigned char* image;

    char buffer[100];    
    for(GLuint i = 0; i < 6; i++)
    {
        buffer[0] = '\0';
        strcat(buffer, "../textures/");
        strcat(buffer, faces[i]);
        image = SOIL_load_image(buffer, &width, &height, 0, SOIL_LOAD_RGB);

        if(image == NULL)
            fprintf(stderr, "Failed to load %s.\n", faces[i]);
        
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        SOIL_free_image_data(image);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    return textureID;
}

void initSkybox(Skybox *skybox)
{
    skybox->shaderProgram = compileShaders(skyboxVertSrc, skyboxFragSrc);
    glUseProgram(skybox->shaderProgram);
    
    Mesh cubeMesh;
    cubeMesh.loadOBJFile("cube.obj");
    Geometry *cube = cubeMesh.produceGeometryPNX();    

    glGenVertexArrays(1, &(skybox->vao));
    glBindVertexArray(skybox->vao);   
    skybox->vbo = cube->vbo;
    glBindBuffer(GL_ARRAY_BUFFER, skybox->vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);

    const char *skyboxTextureFiles[6] = {"skybox_right.jpg", "skybox_left.jpg",
                                "skybox_top.jpg", "skybox_bottom.jpg", "skybox_back.jpg", "skybox_front.jpg"};        
    skybox->cubemap = loadCubemap(skyboxTextureFiles);
    
    glUniformMatrix4fv(glGetUniformLocation(skybox->shaderProgram, "uProjMat"), 1, GL_TRUE, &(g_proj[0]));
    skybox->cubemapUniformHandle = glGetUniformLocation(skybox->shaderProgram, "skybox");
    skybox->viewMatHandle = glGetUniformLocation(skybox->shaderProgram, "uViewMat");
    glBindVertexArray(0);
    glUseProgram(0);
}

// Initializes textures that are used in materials
size_t initTextures(MaterialInfo matInfoList[], const size_t matCount, char **&textureFileNames)
{
    char *nonMTLTextures[5] = {"Ship_Diffuse.png", "default.png", "Ship_Normal.png",
                                "Ship2_Diffuse.png", "Ship2_Normal.png"};

    size_t numNonMTL = 5;
    
    size_t tmp = 5 + matCount;
    textureFileNames = (char**)malloc(sizeof(char*)*tmp);

    for(size_t i = 0; i < tmp; i++)
        textureFileNames[i] = (char*)malloc(sizeof(char)*20);
    
    for(size_t i = 0; i < numNonMTL; i++)
        strcpy(textureFileNames[i], nonMTLTextures[i]);


    size_t tfIndex = 0;
    for(size_t i = 0; i < matCount; i++)
    {
        if(matInfoList[i].map_Kd[0] != '\0')
        {
            bool duplicate = false;
            for(size_t j = 0; j < numNonMTL + tfIndex; j++)
            {
                if(strcmp(matInfoList[i].map_Kd, textureFileNames[j]) == 0)
                    duplicate = true;
            }

            if(!duplicate)
            {
                strcpy(textureFileNames[numNonMTL + tfIndex], matInfoList[i].map_Kd);
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
        char buffer[100];
        buffer[0] = '\0';
        strcat(buffer, "../textures/");
        strcat(buffer, textureFileNames[i]);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        loadAndSpecifyTexture(buffer);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    return numTextureFiles;
}

// Loads the data from MTL files specified by MTLFileNames into matInfoList
size_t loadMTLFiles(MaterialInfo matInfoList[], const size_t infoListSize, char MTLFileNames[][20], const size_t numMTLFiles)
{
    MaterialInfo *tmpList = (MaterialInfo*)malloc(sizeof(MaterialInfo)*infoListSize);
    size_t numMat = 0;

    for(size_t i = 0; i < numMTLFiles; i++)
    {
        size_t matCount = parseMTLFile(tmpList, infoListSize, MTLFileNames[i]);

        if(matCount == 0)
        {
            fprintf(stderr, "Error parsing %s.\n", MTLFileNames[i]);
            continue;
        }
        
        if(matCount + numMat >= infoListSize)
        {
            fprintf(stderr, "Not enough space for %s.\n", MTLFileNames[i]);
            continue;
        }

        for(size_t j = 0; j < matCount; j++)
            matInfoList[numMat + j] = tmpList[j];

        numMat += matCount;
    }

    free(tmpList);
    return numMat;
}

void initMaterial(const MaterialInfo *matInfoList, const size_t matCount)
{
    inputHandler.setViewTransform(RigTForm::lookAt(Vec3(10.0f, 10.0f, 8.0f), Vec3(4.0f, 0.0f, 0.0f), Vec3(0, 1, 0)));


    // TODO: what if two materials have the same name?
    g_shipMaterial1 = new Material(normalVertSrc, normalFragSrc, "ShipMaterial1");
    Vec3 color(1.0f, 1.0f, 1.0f);
    g_shipMaterial1->sendUniform3f("uColor", color);
    g_shipMaterial1->sendUniformMat4("uProjMat", g_proj);
    g_shipMaterial1->sendUniformTexture("uTex0", textures[0]);
    g_shipMaterial1->sendUniformTexture("uTex1", textures[2]);
    
    g_shipMaterial2 = new Material(normalVertSrc, normalFragSrc, "ShipMaterial2");
    g_shipMaterial2->sendUniform3f("uColor", color);
    g_shipMaterial2->sendUniformMat4("uProjMat", g_proj);
    g_shipMaterial2->sendUniformTexture("uTex0", textures[3]);
    g_shipMaterial2->sendUniformTexture("uTex1", textures[4]);

    g_teapotMaterial = new Material(basicVertSrc, ADSFragSrc, "TeapotMaterial");
    g_teapotMaterial->sendUniform3f("uColor", color);
    g_teapotMaterial->sendUniformMat4("uProjMat", g_proj);
    g_teapotMaterial->sendUniformTexture("uTex0", textures[1]);

    g_cubeMaterial = new Material(basicVertSrc, diffuseFragSrc, "CubeMaterial");
    g_cubeMaterial->sendUniform3f("uColor", Vec3(1.0f, 1.0f, 0.0f));
    g_cubeMaterial->sendUniformMat4("uProjMat", g_proj);

    g_cubemapReflectionMat = new Material(cubemapReflectionVertSrc, cubemapReflectionFragSrc, "CubemapReflection");
    g_cubemapReflectionMat->sendUniformMat4("uProjMat", g_proj);
    // TODO: change the source of the cubemap to something else that isn't the skybox struct
    g_cubemapReflectionMat->sendUniformCubemap("uCubemap", g_skybox.cubemap);

    // Initialize materials with data in matInfoList
    for(size_t i = 0; i < matCount; i++)
    {
        g_materials[i] = new Material(basicVertSrc, OBJFragSrc, matInfoList[i].name);
        g_materials[i]->sendUniformMat4("uProjMat", g_proj);
        g_materials[i]->sendUniform3f("Ka", matInfoList[i].Ka);
        g_materials[i]->sendUniform3f("Kd", matInfoList[i].Kd);
        g_materials[i]->sendUniform3f("Ks", matInfoList[i].Ks);
        g_materials[i]->sendUniform1f("Ns", matInfoList[i].Ns);

        if(matInfoList[i].name[0] == '\0')
            g_materials[i]->sendUniformTexture("uTex0", textures[1]);
        else
        {
            size_t j;
            for(j = 0; j < g_numTextures; j++)
            {
                if(strcmp(matInfoList[i].map_Kd, g_textureFileNames[j]) == 0)
                    break;
            }

            if(j == g_numTextures)
                g_materials[i]->sendUniformTexture("uTex0", textures[1]);
            else
            {
                g_materials[i]->sendUniformTexture("uTex0", textures[j]);
            }
        }
    }

    g_numMat += matCount;
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
    
    modelRbt = RigTForm(Vec3(0.0f, 0.0f, 0.0f));
    g_cubeNode = new GeometryNode(g_cube, g_cubeMaterial, modelRbt, true);

    modelRbt = RigTForm(Vec3(10.0f, 0.0f, 0.0f));
    //g_teapotNode = new GeometryNode(g_teapot, g_teapotMaterial, modelRbt, true);
    g_teapotNode = new GeometryNode(g_teapot, g_cubemapReflectionMat, modelRbt, true);
    g_teapotNode->setScaleFactor(Vec3(1.0f/15.0f, 1.0f/15.0f, 1.0f/15.0f));

    modelRbt = RigTForm(Vec3(0.0f, 0.0f, 0.0f));
    g_sponzaNode = new MultiGeometryNode(g_geometryGroups, g_groupInfoList[0], g_materials, g_numMat, modelRbt, false);

    g_crysponzaNode = new GeometryNode(g_crysponza, g_teapotMaterial, modelRbt, false);
    g_crysponzaNode->setScaleFactor(Vec3(1.0f/50.0f, 1.0f/50.0f, 1.0f/50.0f));



    g_worldNode->addChild(g_terrainNode);
    //g_worldNode->addChild(g_ship2Node);
    //g_worldNode->addChild(g_cubeNode);
    g_worldNode->addChild(g_teapotNode);
    //g_worldNode->addChild(g_sponzaNode);
    //g_worldNode->addChild(g_crysponzaNode);
}

void initRenderToBuffer(RTB *rtb)
{
    rtb->shaderProgram = compileShaders(RTBVertSrc, RTBFragSrc);
    glUseProgram(rtb->shaderProgram);
    // The quad that covers the whole viewport
    GLfloat vertices[] = {
        -1.0f,  -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0, -1.0, 1.0f, 0.0f,
        -1.0, 1.0, 0.0f, 1.0f,
        1.0, 1.0, 1.0f, 1.0f
    };    

    // Generate and bind buffer objects
    glGenVertexArrays(1, &(rtb->vao));
    glBindVertexArray(rtb->vao);

    glGenBuffers(1, &(rtb->vbo));
    glBindBuffer(GL_ARRAY_BUFFER, rtb->vbo);
    //sizeof works because vertices is an array, not a pointer.
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Setup vertex attributes
    //GLint posAttrib = glGetAttribLocation(RTBProgram, "aPosition");
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

    // Setup texture handle
    rtb->texture = glGetUniformLocation(rtb->shaderProgram, "uTex0");    

    // Generate framebuffer
    glGenFramebuffers(1, &(rtb->framebuffer));
    glBindFramebuffer(GL_FRAMEBUFFER, rtb->framebuffer);

    // Generate texture
    glGenTextures(1, &(rtb->texColorBuffer));
    glBindTexture(GL_TEXTURE_2D, rtb->texColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (int)g_windowWidth, (int)g_windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);    
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach texture to currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rtb->texColorBuffer, 0);

    // Create renderbuffer object for stencil and depth buffer
    glGenRenderbuffers(1, &(rtb->rbo));
    glBindRenderbuffer(GL_RENDERBUFFER, rtb->rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (int)g_windowWidth, (int)g_windowHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rtb->rbo);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "Frame buffer is not complete.\n");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

    // ----------------------------- RESOURCES ----------------------------- //
    int a;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &a);
    printf("Max texture image units = %d\n", a);

    inputHandler.initialize();
    
    initGeometries();
    
    MaterialInfo matInfoList[MAX_MATERIALS];
    MaterialInfo totalMatInfoList[MAX_MATERIALS];

    const size_t numMTLFiles = 1;
    char MTLFileNames[numMTLFiles][20] = {"sponza.mtl"};
    
    size_t matCount = loadMTLFiles(matInfoList, MAX_MATERIALS, MTLFileNames, numMTLFiles);

    g_numTextures = initTextures(matInfoList, matCount, g_textureFileNames);
    g_proj = Mat4::makeProjection(60.0f, g_windowWidth/g_windowHeight, 0.1f, 50.0f);
    initSkybox(&g_skybox);            
    initMaterial(matInfoList, matCount);
    initScene();
    initRenderToBuffer(&g_rtb);

    

    //g_skyboxTexture = loadCubemap(skyboxTextureFiles);

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
    glDeleteBuffers(1, &(g_cube->vbo));
    glDeleteBuffers(1, &(g_ship1->vbo));
    glDeleteBuffers(1, &(g_ship2->vbo));
    glDeleteBuffers(1, &(g_teapot->vbo));
    glDeleteVertexArrays(1, &(g_cube->vao));
    glDeleteVertexArrays(1, &(g_ship1->vao));
    glDeleteVertexArrays(1, &(g_ship2->vao));
    glDeleteVertexArrays(1, &(g_teapot->vao));

    free(g_shipMaterial1);
    free(g_cubeMaterial);
    free(g_cube);
    free(g_ship1);
    free(g_ship2);


    // ---------------------------- TERMINATE ----------------------------- //

    // Terminate GLFW
    glfwTerminate();

    return 0;
}
