#include <GL/glew.h>

#if __GNUG__
#   include <GLFW/glfw3.h>
#else
#   include <GL/glfw3.h>
#endif

#include <iostream>
#include <stdlib.h>
#include <time.h>

#include "SOIL.h"
#include "fileIO.h"
#include "shaders.h"

//#include "scenegraph.h"
#include "input.h"
#include "mesh.h"

/*
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
*/

#define GLSL(src) "#version 150 core\n" #src

static float g_windowWidth = 1280.0f;
static float g_windowHeight = 720.0f;

// some of the shader uniforms
static RigTForm g_view;
static Vec3 g_lightE, g_lightW(0.0f, 9.5f, 5.0f);
//static Vec3 g_lightE, g_lightW(0.07625f, 1.0f, 0.9f);
static Mat4 g_proj;

// Geometries
static Geometry *g_cube, *g_floor, *g_wall, *g_ship1, *g_ship2, *g_terrain, *g_teapot, *g_sponza, *g_crysponza;
static const size_t MAX_GEOMETRY_GROUPS = 1000;
static Geometry *g_geometryGroups[MAX_GEOMETRY_GROUPS];
static size_t g_groupSize = 0;

static GeoGroupInfo g_groupInfoList[MAX_GEOMETRY_GROUPS];
static int g_groupInfoSize = 0;

// Scenegraph nodes
static TransformNode *g_worldNode;
static GeometryNode *g_terrainNode, *g_cubeArray[4], *g_cubeNode, *g_teapotNode, *g_sponzaNode, *g_ship2Node;
static GeometryNode *g_crysponzaNode;


// Materials
static Material *g_shipMaterial1, *g_shipMaterial2, *g_pickMaterial, *g_cubeMaterial, *g_teapotMaterial;
static const int MAX_MATERIALS = 200;
static Material *g_materials[MAX_MATERIALS];
static int g_numMat = 0;


static GLuint *textures;
static char **g_textureFileNames;
static size_t g_numTextures;

GLFWwindow* window;

static double g_framesPerSec = 60.0f;
static double g_distancePerSec = 3.0f;
static double g_timeBetweenFrames = 1.0 / g_framesPerSec;
static double g_distancePerFrame = g_distancePerSec / g_framesPerSec;



// New: InputHanlder
InputHandler inputHandler;

void draw_scene()
{
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //trans = Mat4::makeZRotation((float)(glfwGetTime()*30));
    // Update cube positions
    /*
    for(int i = 0; i < 2; i++)
        g_cubeArray[i]->setRigidBodyTransform(RigTForm(Quat::makeYRotation(1.0f)) *g_cubeArray[i]->getRbt());
    */
    // Calculate camera movement
    RigTForm trans(inputHandler.getMovementDir() * g_distancePerFrame);
    inputHandler.setViewTransform(trans * inputHandler.getViewTransform());

    // NOTE: since visitor already passes the view matrix, let it carry other often updated uniforms too,
    // like g_lightE
    // Update some uniforms    
    g_lightE = inputHandler.getViewTransform() * g_lightW;
    g_shipMaterial1->sendUniform3v("uLight", g_lightE);
    g_shipMaterial2->sendUniform3v("uLight", g_lightE);    
    g_cubeMaterial->sendUniform3v("uLight", g_lightE);
    g_teapotMaterial->sendUniform3v("uLight", g_lightE);
    //Vec3 lightTargetW(1.0f, 0.0f, 0.0f);
    //Vec3 lightTargetE = inputHandler.getViewTransform() * lightTargetW;
    //g_teapotMaterial->sendUniform3v("uLightTarget", lightTargetE);

    // TODO: update the uniforms in g_materials
    for(int i = 0; i < g_numMat; i++)
        g_materials[i]->sendUniform3v("uLight", g_lightE);

    // Draw objects

    Visitor visitor(inputHandler.getViewTransform());
    visitor.visitNode(inputHandler.getWorldNode());

    //RigTForm modelViewRbt = inputHandler.getViewTransform();
    //g_sponzaNode->drawGroup(modelViewRbt);

    glfwSwapBuffers(window);
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



void initGeometry()
{
    GLfloat floor_verts[] = {
        -10.0f, -0.5f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        10.0f, -0.5f, -10.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        10.0f, -0.5f, 10.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -10.0f, -0.5f, 10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };

    GLfloat wall_verts[] = {
        -10.0f, -0.5f, -10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        10.0f, -0.5f, -10.0f, 0.0f, 0.0f, 1.0f, 10.0f, 1.0f,
        10.0f, 1.5f, -10.0f, 0.0f, 0.0f, 1.0f, 10.0f, 0.0f,
        -10.0f, 1.5f, -10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    };

    GLuint elements[] = {
        0, 1, 2,
        0, 2, 3
    };

    Mesh ship1Mesh;
    ship1Mesh.readFromObj("Ship.obj");
    ship1Mesh.computeVertexBasis();
    g_ship1 = ship1Mesh.produceGeometryPNXTBD();

    Mesh ship2Mesh;
    ship2Mesh.readFromObj("Ship2.obj");
    ship2Mesh.computeVertexBasis();
    g_ship2 = ship2Mesh.produceGeometryPNXTBD();

    Mesh cubeMesh;
    cubeMesh.readFromObj("cube.obj");
    g_cube = cubeMesh.produceGeometryPNX();

    Mesh teapotMesh;
    teapotMesh.readFromObj("teapot.obj");
    teapotMesh.computeVertexNormals();
    g_teapot = teapotMesh.produceGeometryPNX();

    Mesh sponzaMesh;
    sponzaMesh.readFromObj("sponza.obj");
    sponzaMesh.computeVertexNormals();
    //sponzaMesh.computeVertexBasis();
    sponzaMesh.flipTexcoordY(); // For some reason the texcoords are flipped for sponza.obj
                                // Or perhaps it's the space ships that have the flipped texcoords
    //g_sponza = sponzaMesh.produceGeometryPNX();
    getGeoList(sponzaMesh, g_geometryGroups, g_groupInfoList, MAX_GEOMETRY_GROUPS, g_groupSize, g_groupInfoSize, PNX);

    Mesh crysponzaMesh;
    crysponzaMesh.readFromObj("crysponza.obj");
    g_crysponza = crysponzaMesh.produceGeometryPNX();
    getGeoList(crysponzaMesh, g_geometryGroups, g_groupInfoList, MAX_GEOMETRY_GROUPS, g_groupSize, g_groupInfoSize, PNX);

    /*
    printf("g_groupSize = %d\n", g_groupSize);
    for(int i = 0; i < g_groupInfoSize; i++)
    {
        printf("offset = %d\n", g_groupInfoList[i].offset);
        printf("numGroups = %d\n", g_groupInfoList[i].numGroups);
        for(int j = 0; j < g_groupInfoList[i].numGroups; j++)
            printf("%s\n", g_groupInfoList[i].mtlNames[j]);
    }
    */
    int vertSizePNX = 8;
    g_floor = new Geometry(floor_verts, elements, 4, 6, vertSizePNX);
    g_wall = new Geometry(wall_verts, elements, 4, 6, vertSizePNX);

    // Terrain
    struct vertex
    {
        float x, y, z;
    };

    vertex grid[20][20];

    // make flat grid
    for(int i = 0; i < 20; i++)
    {
        for(int j = 0; j < 20; j++)
        {
            grid[i][j].x = -10.0f + j;
            grid[i][j].z = 10.0f - i;
            grid[i][j].y = 0.0f;
        }
    }


    {
        // add varying height to each vertex on the grid
        srand(time(NULL));
        for(int i = 0; i < 20; i++)
        {
            for(int j = 0; j < 20; j++)
            {
                int random = rand() % 100 + 1;
                grid[i][j].y += (float)random / 100.0f * 1.0f;
                //grid[i][j].y += (sin(grid[i][j].x) - cos(grid[i][j].z)) * 1.0f;
            }
        }
    
        GLfloat terrain_verts[19*19*2*3*8];
        int index = 0;

        // TODO: change this to index drawing
        for(int i = 0; i < 19; i++)
        {
            for(int j = 0; j < 19; j++)
            {
                // first triangle
                // position
                terrain_verts[index++] = grid[i][j].x;
                terrain_verts[index++] = grid[i][j].y;
                terrain_verts[index++] = grid[i][j].z;
                // normal
                terrain_verts[index++] = 0.0f;
                terrain_verts[index++] = 1.0f;
                terrain_verts[index++] = 0.0f;
                // texcoord
                terrain_verts[index++] = 0.0f;
                terrain_verts[index++] = 0.0f;

                terrain_verts[index++] = grid[i+1][j].x;
                terrain_verts[index++] = grid[i+1][j].y;
                terrain_verts[index++] = grid[i+1][j].z;
                // normal
                terrain_verts[index++] = 0.0f;
                terrain_verts[index++] = 1.0f;
                terrain_verts[index++] = 0.0f;
                // texcoord
                terrain_verts[index++] = 0.0f;
                terrain_verts[index++] = 0.0f;

                terrain_verts[index++] = grid[i+1][j+1].x;
                terrain_verts[index++] = grid[i+1][j+1].y;
                terrain_verts[index++] = grid[i+1][j+1].z;
                // normal
                terrain_verts[index++] = 0.0f;
                terrain_verts[index++] = 1.0f;
                terrain_verts[index++] = 0.0f;
                // texcoord
                terrain_verts[index++] = 0.0f;
                terrain_verts[index++] = 0.0f;

                // second triangle
                terrain_verts[index++] = grid[i][j].x;
                terrain_verts[index++] = grid[i][j].y;
                terrain_verts[index++] = grid[i][j].z;
                // normal
                terrain_verts[index++] = 0.0f;
                terrain_verts[index++] = 1.0f;
                terrain_verts[index++] = 0.0f;
                // texcoord
                terrain_verts[index++] = 0.0f;
                terrain_verts[index++] = 0.0f;

                terrain_verts[index++] = grid[i+1][j+1].x;
                terrain_verts[index++] = grid[i+1][j+1].y;
                terrain_verts[index++] = grid[i+1][j+1].z;
                // normal
                terrain_verts[index++] = 0.0f;
                terrain_verts[index++] = 1.0f;
                terrain_verts[index++] = 0.0f;
                // texcoord
                terrain_verts[index++] = 0.0f;
                terrain_verts[index++] = 0.0f;

                terrain_verts[index++] = grid[i][j+1].x;
                terrain_verts[index++] = grid[i][j+1].y;
                terrain_verts[index++] = grid[i][j+1].z;
                // normal
                terrain_verts[index++] = 0.0f;
                terrain_verts[index++] = 1.0f;
                terrain_verts[index++] = 0.0f;
                // texcoord
                terrain_verts[index++] = 0.0f;
                terrain_verts[index++] = 0.0f;
            }
        }

        g_terrain = new Geometry(terrain_verts, index/8, vertSizePNX);
    }
    //free(mesh_verts);
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
    //printf("fileName = %s\n", fileName);

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

size_t initTexture(MaterialInfo *matInfoList, const size_t matCount, char **&textureFileNames)
{
    char *nonMTLTextures[5] = {"Ship_Diffuse.png", "default.png", "Ship_Normal.png",
                           "Ship2_Diffuse.png", "Ship2_Normal.png"};
    size_t numNonMTL = 5;
    
    size_t tmp = 5 + matCount;
    printf("tmp = %d\n", tmp);
    textureFileNames = (char**)malloc(sizeof(char*)*tmp);

    for(size_t i = 0; i < tmp; i++)
        textureFileNames[i] = (char*)malloc(sizeof(char)*30);
    
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
        
    // Create 2 textures and load images to them
    textures = (GLuint*)malloc(sizeof(GLuint)*numTextureFiles);
    glGenTextures(numTextureFiles, textures);


    for(size_t i = 0; i < numTextureFiles; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        loadAndSpecifyTexture(textureFileNames[i]);
    }

    return numTextureFiles;
}

size_t loadMTLFiles(MaterialInfo matInfoList[], const size_t infoListSize, char MTLFileNames[][20], const size_t numMTLFiles)
{
    MaterialInfo *tmpList = (MaterialInfo*)malloc(sizeof(infoListSize));
    size_t numMat = 0;

    for(size_t i = 0; i < numMTLFiles; i++)
    {
        size_t matCount = parseMTLFile(tmpList, infoListSize, MTLFileNames[i]);
        if(matCount + numMat >= infoListSize)
        {
            fprintf(stderr, "Too many materials.\n");
            return 0;
        }

        for(size_t j = 0; j < matCount; j++)
            matInfoList[numMat + j] = tmpList[j];

        numMat += matCount;
    }

    return numMat;
}

void initMaterial(const MaterialInfo *matInfoList, const size_t matCount)
{
    inputHandler.setViewTransform(RigTForm::lookAt(Vec3(10.0f, 10.0f, 8.0f), Vec3(4.0f, 0.0f, 0.0f), Vec3(0, 1, 0)));

    g_proj = Mat4::makeProjection(60.0f, g_windowWidth/g_windowHeight, 0.1f, 50.0f);
    Mat4 proj = transpose(g_proj);

    // Material
    // TODO: what if two materials have the same name?
    g_shipMaterial1 = new Material(normalVertSrc, normalFragSrc, "ShipMaterial1");
    Vec3 color(1.0f, 1.0f, 1.0f);
    g_shipMaterial1->sendUniform3v("uColor", color);
    g_shipMaterial1->sendUniformMat4("uProjMat", proj);
    g_shipMaterial1->sendUniformTexture("uTex0", textures[0], GL_TEXTURE0, 0);
    g_shipMaterial1->sendUniformTexture("uTex1", textures[2], GL_TEXTURE2, 2);
    
    g_shipMaterial2 = new Material(normalVertSrc, normalFragSrc, "ShipMaterial2");
    g_shipMaterial2->sendUniform3v("uColor", color);
    g_shipMaterial2->sendUniformMat4("uProjMat", proj);
    g_shipMaterial2->sendUniformTexture("uTex0", textures[3], GL_TEXTURE3, 3);
    g_shipMaterial2->sendUniformTexture("uTex1", textures[4], GL_TEXTURE4, 4);

    g_teapotMaterial = new Material(basicVertSrc, ADSFragSrc, "TeapotMaterial");
    g_teapotMaterial->sendUniform3v("uColor", color);
    g_teapotMaterial->sendUniformMat4("uProjMat", proj);
    g_teapotMaterial->sendUniformTexture("uTex0", textures[1], GL_TEXTURE1, 1);

    g_cubeMaterial = new Material(basicVertSrc, diffuseFragSrc, "CubeMaterial");
    g_cubeMaterial->sendUniform3v("uColor", Vec3(1.0f, 1.0f, 0.0f));
    g_cubeMaterial->sendUniformMat4("uProjMat", proj);

    for(size_t i = 0; i < matCount; i++)
    {
        g_materials[i] = new Material(basicVertSrc, OBJFragSrc, matInfoList[i].name);
        g_materials[i]->sendUniformMat4("uProjMat", proj);
        g_materials[i]->sendUniform3v("Ka", matInfoList[i].Ka);
        g_materials[i]->sendUniform3v("Kd", matInfoList[i].Kd);
        g_materials[i]->sendUniform3v("Ks", matInfoList[i].Ks);
        g_materials[i]->sendUniform1f("Ns", matInfoList[i].Ns);

        if(matInfoList[i].name[0] == '\0')
            g_materials[i]->sendUniformTexture("uTex0", textures[1], GL_TEXTURE1, 1);
        else
        {
            size_t j;
            for(j = 0; j < g_numTextures; j++)
            {
                if(strcmp(matInfoList[i].map_Kd, g_textureFileNames[j]) == 0)
                    break;
            }
            g_materials[i]->sendUniformTexture("uTex0", textures[j], GL_TEXTURE0 + j, j);
        }
    }

    g_numMat += matCount;
}

void initScene()
{
    g_worldNode = new TransformNode();
    // Remove this
    inputHandler.setWorldNode(g_worldNode);

    RigTForm modelRbt;
    modelRbt = RigTForm(Vec3(-6.0f, 0.0f, 1.0f));
    g_terrainNode = new GeometryNode(g_ship1, g_shipMaterial1, modelRbt, true);

    modelRbt = RigTForm(Vec3(0.0f, 0.0f, -10.0f));
    g_ship2Node = new GeometryNode(g_ship2, g_shipMaterial2, modelRbt, true);
    
    modelRbt = RigTForm(Vec3(5.0f, 0.0f, 0.0f));
    g_cubeNode = new GeometryNode(g_cube, g_cubeMaterial, modelRbt, true);

    modelRbt = RigTForm(Vec3(10.0f, 0.0f, 0.0f));
    g_teapotNode = new GeometryNode(g_teapot, g_teapotMaterial, modelRbt, true);
    g_teapotNode->setScaleFactor(Vec3(1.0f/15.0f, 1.0f/15.0f, 1.0f/15.0f));

    modelRbt = RigTForm(Vec3(0.0f, 0.0f, 0.0f));
    //g_sponzaNode = new GeometryNode(g_sponza, g_cubeMaterial, modelRbt, false);
    g_sponzaNode = new GeometryNode(g_geometryGroups, g_groupInfoList[0], g_materials, g_numMat, modelRbt, false);

    g_crysponzaNode = new GeometryNode(g_crysponza, g_teapotMaterial, modelRbt, false);
    g_crysponzaNode->setScaleFactor(Vec3(1.0f/50.0f, 1.0f/50.0f, 1.0f/50.0f));
    

    //g_worldNode->addChild(g_terrainNode);
    //g_worldNode->addChild(g_ship2Node);
    //g_worldNode->addChild(g_cubeNode);
    //g_worldNode->addChild(g_teapotNode);
    g_worldNode->addChild(g_sponzaNode);
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
    
    // Create a rendering window with OpenGL 3.2 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
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

    inputHandler.initialize();
    
    initGeometry();
    MaterialInfo matInfoList[MAX_MATERIALS];
    MaterialInfo totalMatInfoList[MAX_MATERIALS];

    const size_t numMTLFiles = 1;
    char MTLFileNames[numMTLFiles][20] = {"sponza.mtl"};
    
    size_t matCount = loadMTLFiles(matInfoList, MAX_MATERIALS, MTLFileNames, numMTLFiles);
    //size_t matCount = parseMtlFile("sponza.mtl", matInfoList);
    /*
    printf("sponza materials\n");
    for(size_t i = 0; i < matCount; i++)
    {
        printf("Material Name: %s\n", matInfoList[i].name);
        printf("Ka: %f, %f, %f\n", matInfoList[i].Ka[0], matInfoList[i].Ka[1], matInfoList[i].Ka[2]);
        printf("Kd: %f, %f, %f\n", matInfoList[i].Kd[0], matInfoList[i].Kd[1], matInfoList[i].Kd[2]);
        printf("Ks: %f, %f, %f\n", matInfoList[i].Ks[0], matInfoList[i].Ks[1], matInfoList[i].Ks[2]);
        printf("Ke: %f, %f, %f\n", matInfoList[i].Ke[0], matInfoList[i].Ke[1], matInfoList[i].Ke[2]);
        printf("Ns: %f\n", matInfoList[i].Ns);
        printf("Ni: %f\n", matInfoList[i].Ni);        
        printf("map_Ka: %s\n", matInfoList[i].map_Ka);
        printf("map_Kd: %s\n", matInfoList[i].map_Kd);
        printf("map_bump: %s\n", matInfoList[i].map_bump);
    }
    */

    g_numTextures = initTexture(matInfoList, matCount, g_textureFileNames);
    for(size_t i = 0; i < g_numTextures; i++)
        printf("%s\n", g_textureFileNames[i]);
    initMaterial(matInfoList, matCount);
    for(size_t i = 0; i < matCount; i++)
        printf("%s\n", g_materials[i]->getName());
    initScene();


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
                timeLastRender = currentTime;
                draw_scene();
            }
        }

        // Poll window events
        glfwPollEvents();

    }

    // ---------------------------- Clearing ------------------------------ //

    // Delete allocated resources


    glDeleteTextures(2, textures);
    glDeleteBuffers(1, &(g_cube->vbo));
    glDeleteBuffers(1, &(g_floor->vbo));
    glDeleteBuffers(1, &(g_wall->vbo));
    glDeleteBuffers(1, &(g_ship1->vbo));
    glDeleteBuffers(1, &(g_ship2->vbo));
    glDeleteBuffers(1, &(g_teapot->vbo));
    glDeleteVertexArrays(1, &(g_cube->vao));
    glDeleteVertexArrays(1, &(g_floor->vao));
    glDeleteVertexArrays(1, &(g_wall->vao));
    glDeleteVertexArrays(1, &(g_ship1->vao));
    glDeleteVertexArrays(1, &(g_ship2->vao));
    glDeleteVertexArrays(1, &(g_teapot->vao));

    //free(flatShader);
    //free(texturedShader);
    free(g_shipMaterial1);
    free(g_cubeMaterial);
    free(g_cube);
    free(g_ship1);
    free(g_ship2);
    free(g_floor);
    free(g_wall);


    // ---------------------------- TERMINATE ----------------------------- //

    // Terminate GLFW
    glfwTerminate();

    return 0;
}
