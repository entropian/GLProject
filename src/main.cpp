#include <GL/glew.h>

#if __GNUG__
#   include <GLFW/glfw3.h>
#else
#   include <GL/glfw3.h>
#endif

#include <iostream>
#include <stdlib.h>
#include <time.h>

#include "fileIO.h"

//#include "scenegraph.h"
#include "input.h"

/*
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
*/

#define GLSL(src) "#version 150 core\n" #src

GLint uniView1, uniView2;

static double cursorX;
static double cursorY;

static int g_windowWidth = 800;
static int g_windowHeight = 600;

// some of the shader uniforms
static RigTForm g_view;
static Vec3 g_lightE, g_lightW(0.0f, 10.0f, 5.0f);
static Mat4 g_proj;

static Geometry *g_cube, *g_floor, *g_wall, *g_mesh, *g_terrain, *g_arrow;
//static ShaderState *flatShader, *texturedShader;
static TransformNode *g_worldNode;
static GeometryNode *g_terrainNode, *g_cubeArray[4], *g_cubeNode, *g_arrowYNode, *g_arrowXNode, *g_arrowZNode;

//test
static Material *shipMaterial1, *pickMaterial, *cubeMaterial;
static Material *arrowYMat, *arrowZMat, *arrowXMat;

GLint uniTrans1, uniTrans2;
GLuint textures[2];
GLFWwindow* window;

static double framesPerSec = 60.0f;
static double distancePerSec = 2.0f;
static double timeBetweenFrames = 1.0 / framesPerSec;
static double distancePerFrame = distancePerSec / framesPerSec;




// New: InputHanlder
InputHandler inputHandler;


//////////////// Shaders
//////// Vertex Shaders
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

static const char *pickVertSrc = GLSL(
    uniform mat4 uModelViewMat;
    uniform mat4 uProjMat;

    in vec3 aPosition;
    in vec3 aNormal;
    in vec2 aTexcoord;

    void main()
    {
        gl_Position = uProjMat * uModelViewMat * vec4(aPosition, 1.0);
    }
);

static const char *pickFragSrc = GLSL(
    uniform int uCode;

    out vec4 outColor;
    
    void main()
    {
        float color = uCode / 255.0;
        //float color = 1.0;
        outColor = vec4(color, color, color, 1.0);
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

static const char* flatFragSrc = GLSL(
    uniform vec3 uColor;
      
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;

    out vec4 outColor;

    void main()
    {
        outColor = vec4(uColor, 1.0);
    }
);

static const char* basicFragSrc = GLSL(
    uniform vec3 uLight;
    uniform vec3 uColor;
    uniform sampler2D uTex0;
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;

    out vec4 outColor;

    void main()
    {

        vec3 lightDir = normalize(uLight - vPosition);
        vec4 texColor = texture(uTex0, vTexcoord) * vec4(uColor, 1.0);
        outColor = vec4((dot(lightDir, vNormal) * texColor.xyz), 1.0);

        //outColor = texture(uTex0, vTexcoord) * vec4(uColor, 1.0);
        //outColor = vec4(uColor, 1.0);
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

    // Setup additional uniforms

    //flatShader->sendLightEyePos(g_lightE);
    //flatShader->sendColor(Vec3(0.1f, 0.6f, 0.6f));

    // Calculate camera movement
    RigTForm trans(inputHandler.getMovementDir() * distancePerFrame);
    inputHandler.setViewTransformation(trans * inputHandler.getViewTransformation());

    // TODO: since visitor already passes the view matrix, let it carry other often updated uniforms too,
    // like g_lightE
    // Update some uniforms    
    g_lightE = inputHandler.getViewTransformation() * g_lightW;
    shipMaterial1->sendUniform3v("uLight", g_lightE);
    cubeMaterial->sendUniform3v("uLight", g_lightE);

    // Draw objects
    Visitor visitor(inputHandler.getViewTransformation());
    visitor.visitNode(inputHandler.getWorldNode());

    glfwSwapBuffers(window);
}

void setArrowsClickable()
{
    g_arrowYNode->setClickable(true);
    g_arrowXNode->setClickable(true);
    g_arrowZNode->setClickable(true);
}

void setArrowsUnclickable()
{
    g_arrowYNode->setClickable(false);
    g_arrowXNode->setClickable(false);
    g_arrowZNode->setClickable(false);
}

void setClickCoordinate(double x, double y)
{
    inputHandler.setClickX(x);
    inputHandler.setClickY(y);
}

void calcPickedArrow()
{
    printf("picking arrow\n");
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Visitor visitor(inputHandler.getViewTransformation());
    // Draws the picking frame to the backbuffer
    visitor.visitPickNode(g_worldNode, pickMaterial);

    // Determine which object is picked
    unsigned char pixel[3];
    glReadPixels(cursorX, g_windowHeight - cursorY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &pixel);
    printf("pixel[0] == %d\n", pixel[0]);

    inputHandler.setPickedArrow(visitor.getClickedNode(pixel[0] - 1));

    if((inputHandler.getPickedArrow() == g_arrowYNode || inputHandler.getPickedArrow() == g_arrowZNode) ||
       inputHandler.getPickedArrow() == g_arrowXNode)
        setClickCoordinate(cursorX, cursorY);
    else
        inputHandler.setPickedArrow(NULL);

}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return;

    if(inputHandler.getInputMode() == FPS_MODE)
    {
        if (action == GLFW_PRESS)
        {
        
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwGetCursorPos(window, &cursorX, &cursorY);
        }
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }else if(inputHandler.getInputMode() == PICKING_MODE)
    {
        if(action == GLFW_PRESS)
        {
            glfwGetCursorPos(window, &cursorX, &cursorY);
        }
        else if(action == GLFW_RELEASE)
        {
            // TODO: Get the mouse coordinate, and determine which object the user selected.
            inputHandler.setPickModeClicked(true);
        }
    }else if(inputHandler.getInputMode() == OBJECT_MODE)
    {

        if(action == GLFW_PRESS)
        {
            // TODO: draw a frame to backbuffer for picking
            if(inputHandler.getPickedArrow() == NULL)
            {

                setArrowsClickable();
                calcPickedArrow();

                if(inputHandler.getPickedArrow() == g_arrowYNode)
                    printf("Y arrow.\n");
                else if(inputHandler.getPickedArrow() == g_arrowXNode)
                    printf("X arrow.\n");
                else if(inputHandler.getPickedArrow() == g_arrowZNode)
                    printf("Z arrow.\n");

            }else
            {
                glfwGetCursorPos(window, &cursorX, &cursorY);
                //dragArrow(g_clickX, g_clickY, cursorX, cursorY);
            }

        }else if(action == GLFW_RELEASE)
        {
            // TODO: use mouse release coordinate to determine
            // if one of the arrows was clicked
            if(inputHandler.getPickedArrow() != NULL)
            {
                setArrowsUnclickable();
                inputHandler.setPickedArrow(NULL);
            }
        }
    }

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


        RigTForm tform = RigTForm(Quat::makeYRotation(dx * 0.5f)) * RigTForm(Quat::makeXRotation(dy * 0.5f));
        inputHandler.setViewTransformation(tform * inputHandler.getViewTransformation());
        // Keep the camera upright
        // Rotate the camera around its z axis so that its y axis is
        // in the plane defined by its z axis and world y axis.
        Vec3 newUp = inputHandler.getViewTransformation().getRotation() * Vec3(0, 1, 0);
        Vec3 newX;
        if(abs(dot(newUp, Vec3(0.0f, 0.0f, -1.0f))) > 0.999f)
            newX = Vec3(0.0f, 0.0f, -1.0f);
        else
            newX = normalize(cross(Vec3(0, 0, -1), newUp));
        float halfAngle = (float)acos(dot(newX, Vec3(1, 0, 0))) / 2.0f;
        float sign = 1;
        // Not sure why this works
        if(newUp[0] < 0.0f)
            sign = -1;
        Quat q(cos(halfAngle), 0, 0, sin(halfAngle)*sign);
        tform = RigTForm(Vec3(0, 0, 0), q);
        inputHandler.setViewTransformation(tform * inputHandler.getViewTransformation());

        // Report camera position in world space
        /*
        RigTForm invView = inv(g_view);
        Vec3 trans = invView.getTranslation();        
        std::cout << "Camera pos: " << trans[0] << " " << trans[1] << " " << trans[2] << "\n";
        */
        

    }
    
    cursorX = x;
    cursorY = y;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    /*
    if(action != GLFW_PRESS && action != GLFW_REPEAT)
        return;
    */
    /*
      BUG: program crashes when pressing P after picking at background
    */
    /*
    if(key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        if(inputHandler.getInputMode() == OBJECT_MODE)
            removeArrows(inputHandler.getPickedObj());
        // TODO: Render the pickable objects to the back buffer.
        if(inputHandler.getInputMode() != PICKING_MODE)
        {
            printf("Switching to picking mode\n");
            inputHandler.setPrevInputMode(inputHandler.getInputMode());
            inputHandler.setInputMode(PICKING_MODE);  // Stops the rendering loop

            calcPickedObj();
        }else
        {
            inputHandler.setInputMode(inputHandler.getPrevInputMode());
        } 
    }else if(key == GLFW_KEY_L && action == GLFW_PRESS)
    {
        if(inputHandler.getInputMode() == OBJECT_MODE)
            removeArrows(inputHandler.getPickedObj());
        inputHandler.setPickedObj(NULL);
        inputHandler.setInputMode(FPS_MODE);
    }else if(inputHandler.getInputMode() == FPS_MODE)
    {
        FPSModeKeyInput(key, action);
    }else if(inputHandler.getInputMode() == OBJECT_MODE)
    {
        ObjModeKeyInput(key, action);
    }
    */
    inputHandler.handleKey(window, key, scancode, action, mods);
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

    GLfloat *mesh_verts, *arrow_verts;
    int numVertices;

    mesh_verts = readFromObj("Ship.obj", &numVertices);
    g_mesh = new Geometry(mesh_verts, numVertices);
    
    arrow_verts = readFromObj("arrow.obj", &numVertices);
    g_arrow = new Geometry(arrow_verts, numVertices);
    
    g_cube = new Geometry(vertices, 36);


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
    free(arrow_verts);
}

void initTexture()
{
    // Create 2 textures and load images to them    
    glGenTextures(2, textures);
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

}

void initMaterial()
{
    //flatShader = new ShaderState(vertexSource, fragmentSource);
    //flatShader = new ShaderState(diffuseVertSrc, fragmentSource);
    //flatShader = new ShaderState(basicVertSrc, diffuseFragSrc);
    //flatShader = new ShaderState(basicVertSrc, specularFragSrc);
    //texturedShader = new ShaderState(vertexSource, floorFragSrc);
    //texturedShader = new ShaderState(lightVertexSrc, floorFragSrc);

    inputHandler.setViewTransformation(RigTForm::lookAt(Vec3(0.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0, 1, 0)));

    g_proj = Mat4::makeProjection(60.0f, 800.0f/600.0f, 0.1f, 30.0f);
    Mat4 proj = transpose(g_proj);
    g_lightE = inputHandler.getViewTransformation() * g_lightW;

    // Material
    //material = new Material(basicVertSrc, diffuseFragSrc);
    shipMaterial1 = new Material(basicVertSrc, basicFragSrc);
    //Vec3 color(0.1f, 0.6f, 0.6f);
    Vec3 color(1.0f, 1.0f, 1.0f);
    shipMaterial1->sendUniform3v("uColor", color);
    shipMaterial1->sendUniformMat4("uProjMat", proj);
    shipMaterial1->sendUniformTexture("uTex0", textures[1], GL_TEXTURE1, 1);
    //material->sendUniform3v("uLight", g_lightE);

    cubeMaterial = new Material(basicVertSrc, diffuseFragSrc);
    cubeMaterial->sendUniform3v("uColor", Vec3(1.0f, 1.0f, 0.0f));
    cubeMaterial->sendUniformMat4("uProjMat", proj);

    arrowYMat = new Material(basicVertSrc, flatFragSrc);
    arrowYMat->sendUniform3v("uColor", Vec3(0.0f, 0.0f, 1.0f));
    arrowYMat->sendUniformMat4("uProjMat", proj);

    arrowZMat = new Material(basicVertSrc, flatFragSrc);
    arrowZMat->sendUniform3v("uColor", Vec3(0.0f, 1.0f, 0.0f));
    arrowZMat->sendUniformMat4("uProjMat", proj);

    arrowXMat = new Material(basicVertSrc, flatFragSrc);
    arrowXMat->sendUniform3v("uColor", Vec3(1.0f, 0.0f, 0.0f));
    arrowXMat->sendUniformMat4("uProjMat", proj);

    pickMaterial = new Material(pickVertSrc, pickFragSrc);
    pickMaterial->sendUniformMat4("uProjMat", proj);
    // Remove this
    inputHandler.setPickMaterial(pickMaterial);
}

void initScene()
{
    g_worldNode = new TransformNode();
    // Remove this
    inputHandler.setWorldNode(g_worldNode);

    RigTForm modelRbt;
    //modelRbt = RigTForm(Vec3(-6.0f, 0.0f, 0.0f));
    modelRbt = RigTForm(Vec3(0.0f, 0.0f, 0.0f));
    g_terrainNode = new GeometryNode(modelRbt, g_mesh, shipMaterial1, true);
    
    modelRbt = RigTForm(Vec3(5.0f, 0.0f, 0.0f));
    g_cubeNode = new GeometryNode(modelRbt, g_cube, cubeMaterial, true);

    modelRbt = RigTForm(Vec3(0.0f, 0.0f, 0.0f));
    g_arrowYNode = new GeometryNode(modelRbt, g_arrow, arrowYMat, false);
    g_arrowYNode->setDepthTest(false);

    modelRbt = RigTForm(Quat::makeZRotation(-90.0f));
    g_arrowXNode = new GeometryNode(modelRbt, g_arrow, arrowXMat, false);
    g_arrowXNode->setDepthTest(false);

    modelRbt = RigTForm(Quat::makeXRotation(-90.0f));
    g_arrowZNode = new GeometryNode(modelRbt, g_arrow, arrowZMat, false);
    g_arrowZNode->setDepthTest(false);
    
    g_worldNode->addChild(g_terrainNode);
    g_worldNode->addChild(g_cubeNode);

    // Really bad temporary crap
    inputHandler.setArrows(g_arrowYNode, g_arrowXNode, g_arrowZNode);

    /*
    for(int i = 0; i < 2; i++)
    {
        g_cubeArray[i] = new GeometryNode(NULL, RigTForm(), g_cube, flatShader);
        g_worldNode->addChild(g_cubeArray[i]);
    }
    */
    /*
    modelRbt;
    modelRbt = RigTForm(Vec3(2, 0, 0));
    g_cubeArray[0]->setRbt(modelRbt);
    modelRbt = RigTForm(Quat::makeXRotation(90.0f)) * RigTForm(Quat::makeYRotation(90.0f)) * RigTForm(Vec3(2, 0, 0));
    g_cubeArray[1]->setRbt(modelRbt);
    */
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
    window = glfwCreateWindow(g_windowWidth, g_windowHeight, "OpenGL", NULL, NULL);
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
            if((currentTime - timeLastRender) >= timeBetweenFrames)
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
    free(shipMaterial1);
    free(cubeMaterial);
    free(g_cube);
    free(g_mesh);
    free(g_floor);
    free(g_wall);


    // ---------------------------- TERMINATE ----------------------------- //

    // Terminate GLFW
    glfwTerminate();

    return 0;
}
