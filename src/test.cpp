#include <GL/glew.h>
#include <GL/glfw3.h>
#include "SOIL.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

#include "vec.h"
#include "mat.h"
#include "quat.h"
#include "rigtform.h"
#include "geometry.h"
#include "material.h"
#include "scenegraph.h"

/*
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
*/

#define GLSL(src) "#version 150 core\n" #src

GLint uniView1, uniView2;

static double cursorX;
static double cursorY;

static RigTForm g_view, g_trans;
static Vec3 g_lightE, g_lightW(0.0f, 10.0f, 5.0f);
static Mat4 g_proj;
static Geometry *g_cube, *g_floor, *g_wall, *g_mesh, *g_terrain;
static ShaderState *flatShader, *texturedShader;
static TransformNode *g_worldNode;
static GeometryNode *g_terrainNode, *g_cubeArray[4];

//test
static Material *material;

GLint uniTrans1, uniTrans2;
GLuint textures[2];
GLFWwindow* window;


//////////////// Shaders
//////// Vertex Shaders
static const char* vertexSource = GLSL(
    uniform mat4 uModelView;
    uniform mat4 uproj;
    uniform vec3 color;

    in vec3 position;
    in vec3 normal;
    in vec2 texcoord;

    out vec3 Color;
    out vec2 Texcoord;
    out vec3 Position;
        
    void main() {
        Color = color;
        Texcoord = texcoord;
        Position = position;
        gl_Position = proj * view * trans * vec4(position, 1.0);
    }
);

static const char* basicVertSrc = GLSL(
    uniform mat4 uModelViewMat;
    uniform mat4 uNormalMat;
    uniform mat4 uProjMat;

    in vec3 aPosition;
    in vec3 aNormal;
    in vec2 aTexcoord;

    out vec3 vPosition;
    out vec3 vNormal;
    out vec2 vTexcoord;

    void main()
    {
        vPosition = (uModelViewMat * vec4(aPosition, 1.0)).xyz;
        vNormal = (uNormalMat * vec4(aNormal, 1.0)).xyz;
        vTexcoord = aTexcoord;
        gl_Position = uProjMat * vec4(vPosition, 1.0);
    }
);

// TODO: why doesn't this work anymore?
// the most primitive shader with lighting
static const char* diffuseVertSrc = GLSL(
    uniform mat4 uModelViewMat;
    uniform mat4 uNormalMat;
    uniform mat4 uProjMat;
    uniform vec3 uColor;
    // uLight is in eye space
    uniform vec3 uLight;

    in vec3 aPosition;
    in vec3 aNormal;
    in vec2 aTexcoord;

    out vec3 vColor;
    out vec2 vTexcoord;
        
    void main() {
        // in world space
        /*
        vec3 posW = (trans * vec4(position, 1.0)).xyz;
        vec3 normW = (transpose(inverse(trans)) * vec4(normal, 1.0)).xyz;
        vec3 lightDir = normalize(light - posW);
        */

        vec3 posE = (uModelViewMat * vec4(aPosition, 1.0)).xyz;
        //vec3 normE = (inverse(transpose(uModelViewMat)) * vec4(aNormal, 1.0)).xyz;
        vec3 normE = (normalMat * vec4(aNormal, 1.0)).xyz;
        vec3 lightDirE = normalize(uLight - posE);

        if(dot(lightDirE, normE) > 0)
        {
            vColor = uColor * dot(lightDirE, normE);
        }else
        {
            vColor = vec3(0, 0, 0);
        }

        vTexcoord = aTexcoord;

        // old
        // gl_position = proj * view * trans * vec4(position, 1.0);
        // TODO: switch in vPosition to reduce the number of calculations
        gl_Position = uProjMat * uModelViewMat * vec4(aPosition, 1.0);
    }
);

// slightly better shader with lighting that accounts for the camera position
static const char* lightVertexSrc = GLSL(
    uniform mat4 uModelViewMat;
    uniform mat4 uNormalMat;
    uniform mat4 uProjMat;
    uniform vec3 uColor;
    // uLight is in eye space
    uniform vec3 uLight;

    in vec3 aPosition;
    in vec3 aNormal;
    in vec2 aTexcoord;

    out vec3 vColor;
    out vec2 vTexcoord;
        
    void main() {
        // in eye space
        vec3 posE = (uModelViewMat * vec4(aPosition, 1.0)).xyz;
        vec3 normE = (uNormalMat * vec4(aNormal, 1.0)).xyz;
        vec3 lightDirE = normalize(uLight - posE);
        vec3 eyeDirE = normalize(-posE);
        vec3 reflectDirE = 2.0*dot(lightDirE, normE)*normE - lightDirE;

        if(dot(eyeDirE, reflectDirE) > 0)
        {
            vColor = uColor * dot(eyeDirE, reflectDirE);
        }else
        {
            vColor = vec3(0, 0, 0);
        }

        vTexcoord = aTexcoord;
        gl_Position = uProjMat * uModelViewMat * vec4(aPosition, 1.0);
    }
);

//////// Fragment Shaders
static const char* fragmentSource = GLSL(
    uniform sampler2D texKitten;
    uniform sampler2D texPuppy;

    in vec3 vColor;
    in vec2 vTexcoord;

    out vec4 outColor;
        
    void main() {
        //outColor = mix(texture(texKitten, Texcoord), texture(texPuppy, Texcoord), 0.5) * vec4(Color, 1.0);
        outColor = vec4(vColor, 1.0);
    }
);

static const char* diffuseFragSrc = GLSL(
    uniform vec3 uLight;
    uniform vec3 uColor;
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec3 vTexcoord;

    out vec4 outColor;

    void main()
    {
        vec3 lightDir = normalize(uLight - vPosition);
        outColor = vec4((dot(lightDir, vNormal) * uColor), 1.0);
    }
);

static const char* specularFragSrc = GLSL(
    uniform vec3 uLight;
    uniform vec3 uColor;
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec3 vTexcoord;

    out vec4 outColor;

    void main()
    {
        vec3 lightDir = normalize(uLight - vPosition);
        vec3 reflectDir = 2*dot(lightDir, vNormal)*vNormal - lightDir;
        vec3 eyeDir = normalize(-vPosition);
        outColor = vec4((dot(reflectDir, eyeDir) * uColor), 1.0);
    }
);
 
// Create and compile the fragment shader
static const char* floorFragSrc = GLSL(
    uniform sampler2D texKitten;
    uniform sampler2D texPuppy;

    in vec3 vColor;
    in vec2 vTexcoord;

    out vec4 outColor;
        
    void main() {
        //outColor = vec4(Color, 1.0);
        outColor = mix(texture(texKitten, vTexcoord), texture(texPuppy, vTexcoord), 0.5) * vec4(vColor, 1.0);     
    }
);



void draw_scene()
{    
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //trans = transpose(trans);
    //trans = Mat4::makeTranslation(Vec3(-1, 0, 0)) * trans;
    //trans = Mat4::makeZRotation((float)(glfwGetTime()*30));

    // Update cube positions
    for(int i = 0; i < 2; i++)
        g_cubeArray[i]->setRbt(RigTForm(Quat::makeYRotation(1.0f)) *g_cubeArray[i]->getRbt());


    // Setup additional uniforms
    g_lightE = g_view * g_lightW;
    flatShader->sendLightEyePos(g_lightE);
    flatShader->sendColor(Vec3(0.1f, 0.6f, 0.6f));

    // Draw objects
    //g_terrainObject->draw();
    Visitor visitor(g_view);
    visitor.visitNode(g_worldNode);

    /* Floor and walls
    glUseProgram(texturedShader->shaderProgram);
    texturedShader->draw(g_floor);

    texturedShader->draw(g_wall);

    trans = Mat4::makeTranslation(Vec3(-1.0f, 0.0f, 20.0f));
    trans = transpose(trans);
    glUniformMatrix4fv(texturedShader->h_uTrans, 1, GL_FALSE, &(trans[0]));
    texturedShader->draw(g_wall);

    trans = Mat4::makeTranslation(Vec3(-1, 0, 0)) * Mat4::makeYRotation(90.0f);
    trans = transpose(trans);
    glUniformMatrix4fv(texturedShader->h_uTrans, 1, GL_FALSE, &(trans[0]));
    texturedShader->draw(g_wall);

    trans = Mat4::makeTranslation(Vec3(19, 0, 0)) * Mat4::makeYRotation(90.0f);
    trans = transpose(trans);
    glUniformMatrix4fv(texturedShader->h_uTrans, 1, GL_FALSE, &(trans[0]));
    texturedShader->draw(g_wall);
    */
    glfwSwapBuffers(window);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return;

    if (action == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwGetCursorPos(window, &cursorX, &cursorY);
    }
    else
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    /*
    if(button == GLFW_MOUSE_BUTTON_LEFT)
    {
        g_view = RigTForm(Quat::makeYRotation(.1)) * g_view;
        Mat4 view = rigTFormToMat(g_view);
        view = transpose(view);

        glUniformMatrix4fv(uniView, 1, GL_FALSE, &(view[0]));
    }
    */
}

void cursorPosCallback(GLFWwindow* window, double x, double y)
{
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    {
        float dx = (float)(x - cursorX);
        float dy = (float)(y - cursorY);


        RigTForm tform = RigTForm(Quat::makeYRotation(dx)) * RigTForm(Quat::makeXRotation(dy));
        g_view = tform * g_view;
        // Keep the camera upright
        // Rotate the camera around its z axis so that its y axis is
        // in the plane defined by its z axis and world y axis.
        Vec3 newUp = g_view.getRotation() * Vec3(0, 1, 0);
        Vec3 newX = normalize(cross(Vec3(0, 0, -1), newUp));
        float halfAngle = (float)acos(dot(newX, Vec3(1, 0, 0))) / 2.0f;
        float sign = 1;
        // Not sure why this works
        if(newUp[0] < 0.0f)
            sign = -1;
        Quat q(cos(halfAngle), 0, 0, sin(halfAngle)*sign);
        tform = RigTForm(Vec3(0, 0, 0), q);
        g_view = tform * g_view;
        
        RigTForm invView = inv(g_view);
        Vec3 trans = invView.getTranslation();        
        std::cout << "Camera pos: " << trans[0] << " " << trans[1] << " " << trans[2] << "\n";
        
        draw_scene();
        cursorX = x;
        cursorY = y;
    }
}

// TODO: rewrite this to have smooth movement
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action != GLFW_PRESS && action != GLFW_REPEAT)
        return;

    Vec3 movement;
    switch(key)
    {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GL_TRUE);
        break;
    case GLFW_KEY_A:
        movement[0] = .1f;
        break;
    case GLFW_KEY_D:
        movement[0] = -.1f;
        break;
    case GLFW_KEY_W:
        movement[2] = .1f;
        break;
    case GLFW_KEY_S:
        movement[2] = -.1f;
        break;
    default:
        break;
    }

    RigTForm m(movement);
    g_view = m * g_view;
    RigTForm invView = inv(g_view);
    draw_scene();
    Vec3 trans = invView.getTranslation();
    std::cout << "Camera pos: " << trans[0] << " " << trans[1] << " " << trans[2] << "\n";
}

void initGeometry()
{
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

    GLfloat *mesh_verts;
    int numVertices;

    mesh_verts = readFromObj("dish.obj", &numVertices);

    
    g_cube = new Geometry(vertices, 36);
    g_mesh = new Geometry(mesh_verts, numVertices);

    g_floor = new Geometry(floor_verts, elements, 4, 6);
    g_wall = new Geometry(wall_verts, elements, 4, 6);

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

    g_terrain = new Geometry(terrain_verts, index/8);
    free(mesh_verts);
}

void initShader()
{
    //flatShader = new ShaderState(vertexSource, fragmentSource);
    //flatShader = new ShaderState(lightVertexSrc, fragmentSource);
    //flatShader = new ShaderState(diffuseVertSrc, fragmentSource);
    flatShader = new ShaderState(basicVertSrc, diffuseFragSrc);
    //flatShader = new ShaderState(basicVertSrc, specularFragSrc);
    //texturedShader = new ShaderState(vertexSource, floorFragSrc);
    //texturedShader = new ShaderState(lightVertexSrc, floorFragSrc);

    glUseProgram(flatShader->shaderProgram);
    
    g_view = RigTForm::lookAt(Vec3(0.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0, 1, 0));

    g_proj = Mat4::makeProjection(60.0f, 800.0f/600.0f, 0.1f, 30.0f);
    Mat4 proj = transpose(g_proj);
    glUniformMatrix4fv(flatShader->h_uProjMat, 1, GL_FALSE, &(proj[0]));
    glUniform3f(flatShader->h_uColor, 0.1f, 0.6f, 0.6f);
    // transform light to eye space
    g_lightE = g_view * g_lightW;
    glUniform3f(flatShader->h_uLight, g_lightE[0], g_lightE[1], g_lightE[2]);
    
    
    // SECOND SHADER
    /*
    glUseProgram(texturedShader->shaderProgram);
    glUniformMatrix4fv(texturedShader->h_uProjMat, 1, GL_FALSE, &(proj[0]));
    glUniform3f(texturedShader->h_uColor, 0.6f, 0.6f, 0.6f);
    */
    // Material
    material = new Material(basicVertSrc, diffuseFragSrc);
}

void initScene()
{
    RigTForm modelRbt(g_lightW);
    modelRbt = RigTForm(Vec3(0, 0, 0));
    
    g_worldNode = new TransformNode();
    g_terrainNode = new GeometryNode(NULL, modelRbt, g_terrain, flatShader);
    //g_worldNode->addChild(g_terrainNode);

    for(int i = 0; i < 2; i++)
    {
        g_cubeArray[i] = new GeometryNode(NULL, RigTForm(), g_cube, flatShader);
        g_worldNode->addChild(g_cubeArray[i]);
    }

    modelRbt;
    modelRbt = RigTForm(Vec3(2, 0, 0));
    g_cubeArray[0]->setRbt(modelRbt);
    modelRbt = RigTForm(Quat::makeXRotation(90.0f)) * RigTForm(Quat::makeYRotation(90.0f)) * RigTForm(Vec3(2, 0, 0));
    g_cubeArray[1]->setRbt(modelRbt);
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

    window = glfwCreateWindow(800, 600, "OpenGL", NULL, NULL);
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

    initGeometry();
    initShader();
    initScene();


    // Test material
    
    for(GLint i = 0; i < material->numUniforms; i++)
    {
        printf("Name: %s\n", material->uniformDesc[i].name);
        printf("Index: %d\n", material->uniformDesc[i].index);
        switch(material->uniformDesc[i].type)
        {
        case GL_FLOAT_VEC3:
        {
            printf("Type: GL_FLOAT_VEC3\n");
        } break;

        case GL_FLOAT_MAT4:
        {
            printf("Type: GL_FLOAT_MAT4\n");
        } break;
        }
        printf("Size: %d\n", material->uniformDesc[i].size);
    }


    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // ---------------------------- RENDERING ------------------------------ //
    
    glEnable(GL_DEPTH_TEST);
    draw_scene();
    while(!glfwWindowShouldClose(window))
    {

        // Apply a rotation        
        // Swap buffers and poll window events
        //draw_scene();
        glfwPollEvents();
    }

    // ---------------------------- CLEARING ------------------------------ //

    // Delete allocated resources
    glDeleteProgram(flatShader->shaderProgram);
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

    free(flatShader);
    //free(texturedShader);
    free(g_cube);
    free(g_mesh);
    free(g_floor);
    free(g_wall);


    // ---------------------------- TERMINATE ----------------------------- //

    // Terminate GLFW
    glfwTerminate();

    return 0;
}
