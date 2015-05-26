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
static Vec3 g_lightE, g_lightW(0.0f, 10.0f, 5.0f);
static Mat4 g_proj;

// Geometries
static Geometry *g_cube, *g_floor, *g_wall, *g_mesh, *g_terrain, *g_teapot, *g_sponza;
//static ShaderState *flatShader, *texturedShader;
static TransformNode *g_worldNode;
static GeometryNode *g_terrainNode, *g_cubeArray[4], *g_cubeNode, *g_teapotNode, *g_sponzaNode;

//test
static Material *g_shipMaterial1, *g_pickMaterial, *g_cubeMaterial, *g_teapotMaterial;


GLuint textures[4];
GLFWwindow* window;

static double g_framesPerSec = 60.0f;
static double g_distancePerSec = 2.0f;
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

    // TODO: since visitor already passes the view matrix, let it carry other often updated uniforms too,
    // like g_lightE
    // Update some uniforms    
    g_lightE = inputHandler.getViewTransform() * g_lightW;
    g_shipMaterial1->sendUniform3v("uLight", g_lightE);
    g_cubeMaterial->sendUniform3v("uLight", g_lightE);
    g_teapotMaterial->sendUniform3v("uLight", g_lightE);

    // Draw objects
    Visitor visitor(inputHandler.getViewTransform());
    visitor.visitNode(inputHandler.getWorldNode());

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
    // TODO: normals aren't correct?
    GLfloat vertices[] = {
        // front
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

        // back
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.5f, -0.5f,  0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        0.5f,  0.5f,  0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        // left
        -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        // right
        0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        // bottom
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

        // top
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
    };

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

    Mesh shipMesh;
    shipMesh.readFromObj("Ship.obj");
    shipMesh.computeVertexBasis();
    g_mesh = shipMesh.produceGeometryPNXTBD();

    int vertSizePNX = 8;

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
    g_sponza = sponzaMesh.produceGeometryPNX();

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
    //free(mesh_verts);
}

void initTexture()
{
    // Create 2 textures and load images to them    
    glGenTextures(4, textures);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);

    int width, height;
    unsigned char* image;
    image = SOIL_load_image("sample.png", &width, &height, 0, SOIL_LOAD_RGB);
    if(image == NULL)
        fprintf(stderr, "NULL pointer.\n");

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    SOIL_free_image_data(image);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    image = SOIL_load_image("Ship_Diffuse.png", &width, &height, 0, SOIL_LOAD_RGB);
    if(image == NULL)
        fprintf(stderr, "NULL pointer.\n");

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SOIL_free_image_data(image);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    image = SOIL_load_image("default.png", &width, &height, 0, SOIL_LOAD_RGB);
    if(image == NULL)
        fprintf(stderr, "NULL pointer.\n");

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SOIL_free_image_data(image);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textures[3]);
    image = SOIL_load_image("Ship_Normal.png", &width, &height, 0, SOIL_LOAD_RGB);
    if(image == NULL)
        fprintf(stderr, "NULL pointer.\n");

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SOIL_free_image_data(image);

}

void initMaterial()
{
    inputHandler.setViewTransform(RigTForm::lookAt(Vec3(0.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0, 1, 0)));

    g_proj = Mat4::makeProjection(60.0f, g_windowWidth/g_windowHeight, 0.1f, 30.0f);
    Mat4 proj = transpose(g_proj);
    g_lightE = inputHandler.getViewTransform() * g_lightW;

    // Material

    g_shipMaterial1 = new Material(normalVertSrc, normalFragSrc);
    Vec3 color(1.0f, 1.0f, 1.0f);
    g_shipMaterial1->sendUniform3v("uColor", color);
    g_shipMaterial1->sendUniformMat4("uProjMat", proj);
    g_shipMaterial1->sendUniformTexture("uTex0", textures[1], GL_TEXTURE1, 1);
    g_shipMaterial1->sendUniformTexture("uTex1", textures[3], GL_TEXTURE3, 3);

    g_teapotMaterial = new Material(basicVertSrc, ADSFragSrc);
    g_teapotMaterial->sendUniform3v("uColor", color);
    g_teapotMaterial->sendUniformMat4("uProjMat", proj);
    g_teapotMaterial->sendUniformTexture("uTex0", textures[2], GL_TEXTURE2, 2);


    g_cubeMaterial = new Material(basicVertSrc, diffuseFragSrc);
    g_cubeMaterial->sendUniform3v("uColor", Vec3(1.0f, 1.0f, 0.0f));
    g_cubeMaterial->sendUniformMat4("uProjMat", proj);
}

void initScene()
{
    g_worldNode = new TransformNode();
    // Remove this
    inputHandler.setWorldNode(g_worldNode);

    RigTForm modelRbt;
    //modelRbt = RigTForm(Vec3(-6.0f, 0.0f, 0.0f));
    modelRbt = RigTForm(Vec3(-1.0f, 0.0f, -1.0f));
    g_terrainNode = new GeometryNode(modelRbt, g_mesh, g_shipMaterial1, true);
    
    modelRbt = RigTForm(Vec3(5.0f, 0.0f, 0.0f));
    g_cubeNode = new GeometryNode(modelRbt, g_cube, g_cubeMaterial, true);

    modelRbt = RigTForm(Vec3(8.0f, 0.0f, 0.0f));
    g_teapotNode = new GeometryNode(modelRbt, g_teapot, g_teapotMaterial, true);
    g_teapotNode->setScaleFactor(Vec3(1.0f/15.0f, 1.0f/15.0f, 1.0f/15.0f));

    modelRbt = RigTForm(Vec3(5.0f, 0.0f, 0.0f));
    g_sponzaNode = new GeometryNode(modelRbt, g_sponza, g_cubeMaterial, false);
    

    g_worldNode->addChild(g_terrainNode);
    g_worldNode->addChild(g_cubeNode);
    g_worldNode->addChild(g_teapotNode);
    //g_worldNode->addChild(g_sponzaNode);
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

    // ----------------------------- RESOURCES ----------------------------- //

    inputHandler.initialize();
    
    initGeometry();
    initTexture();
    initMaterial();
    initScene();


    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    

    // ---------------------------- RENDERING ------------------------------ //
    
    glEnable(GL_DEPTH_TEST);
    draw_scene();
    double currentTime, timeLastRender = 0;
    while(!glfwWindowShouldClose(window))
    {
        // When g_inputMode == PICKING_MODE, the framebuffer isn't refreshed.
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
    //glDeleteProgram(flatShader->shaderProgram);
    //glDeleteProgram(texturedShader->shaderProgram);
    glDeleteTextures(2, textures);
    glDeleteBuffers(1, &(g_cube->vbo));
    glDeleteBuffers(1, &(g_floor->vbo));
    glDeleteBuffers(1, &(g_wall->vbo));
    glDeleteBuffers(1, &(g_mesh->vbo));
    glDeleteVertexArrays(1, &(g_cube->vao));
    glDeleteVertexArrays(1, &(g_floor->vao));
    glDeleteVertexArrays(1, &(g_wall->vao));
    glDeleteVertexArrays(1, &(g_mesh->vao));

    //free(flatShader);
    //free(texturedShader);
    free(g_shipMaterial1);
    free(g_cubeMaterial);
    free(g_cube);
    free(g_mesh);
    free(g_floor);
    free(g_wall);


    // ---------------------------- TERMINATE ----------------------------- //

    // Terminate GLFW
    glfwTerminate();

    return 0;
}
