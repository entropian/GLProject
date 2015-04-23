#include <GL/glew.h>
#include <GL/glfw3.h>
#include "SOIL.h"
#include <iostream>

#include "vec.h"
#include "mat.h"
#include "quat.h"
#include "rigtform.h"
#include "geometry.h"

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
static Mat4 g_proj;

GLuint textures[2];
GLint uniTrans1, uniTrans2;

GLFWwindow* window;

//up vector


// Create and compile the vertex shader
static const char* vertexSource = GLSL(
    uniform mat4 trans;
    uniform mat4 view;
    uniform mat4 proj;
    uniform vec3 color;

    in vec3 position;
    in vec3 normal;
    in vec2 texcoord;

    out vec3 Color;
    out vec2 Texcoord;
        
    void main() {
        Color = color;
        Texcoord = texcoord;
        gl_Position = proj * view * trans * vec4(position, 1.0);
    }
);

static const char* lightVertexSrc = GLSL(
    uniform mat4 trans;
    uniform mat4 view;
    uniform mat4 proj;
    uniform vec3 color;
    uniform vec3 light;

    in vec3 position;
    in vec3 normal;
    in vec2 texcoord;

    out vec3 Color;
    out vec2 Texcoord;
        
    void main() {
        vec3 lightDir = normalize(light - position);
        mat4 eye = inverse(view);
        vec3 posE = (eye * vec4(0, 0, 0, 1)).xyz;
        //vec3 posE = -(view * trans * vec4(0, 0, 0, 1.0)).xyz;
        //vec3 eyeDir = normalize(posE.xyz);
        vec3 eyeDir = normalize(posE - position);
        vec3 reflectDir = normalize(2*dot(lightDir, normal)*normal - lightDir);
        Color = color * dot(eyeDir, reflectDir);
        Texcoord = texcoord;
        gl_Position = proj * view * trans * vec4(position, 1.0);
    }
);


// Create and compile the fragment shader
static const char* fragmentSource = GLSL(
    uniform sampler2D texKitten;
    uniform sampler2D texPuppy;

    in vec3 Color;
    in vec2 Texcoord;

    out vec4 outColor;
        
    void main() {
        //outColor = mix(texture(texKitten, Texcoord), texture(texPuppy, Texcoord), 0.5) * vec4(Color, 1.0);
        outColor = vec4(Color, 1.0);
    }
);

// Create and compile the fragment shader
static const char* floorFragSrc = GLSL(
    uniform sampler2D texKitten;
    uniform sampler2D texPuppy;

    in vec3 Color;
    in vec2 Texcoord;

    out vec4 outColor;
        
    void main() {
        //outColor = vec4(Color, 1.0);
        outColor = mix(texture(texKitten, Texcoord), texture(texPuppy, Texcoord), 0.5) * vec4(Color, 1.0);     
    }
);

struct ShaderState {
    GLuint shaderProgram;

    // Handles to uniforms
    GLint h_uTrans, h_uView, h_uProj, h_uColor, h_uLight;
    // Handles to attributes
    GLint h_aPosition, h_aNormal, h_aTexcoord;
    

    ShaderState(const char* vs, const char* fs) {
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vs, NULL);
        glCompileShader(vertexShader);

        GLint status;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);

        if(status != GL_TRUE)
            fprintf(stderr, "Vertex shader compiled incorrectly.\n");
        
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fs, NULL);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);

        if(status != GL_TRUE)
            fprintf(stderr, "Fragment shader compiled incorrectly.\n");

        // Link the vertex and fragment shader into a shader program
         shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glBindFragDataLocation(shaderProgram, 0, "outColor");
        glLinkProgram(shaderProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glUseProgram(shaderProgram);

        // Retrieve handles to uniform variables
        h_uProj = glGetUniformLocation(shaderProgram, "proj");
        h_uView = glGetUniformLocation(shaderProgram, "view");
        h_uTrans = glGetUniformLocation(shaderProgram, "trans");
        h_uColor = glGetUniformLocation(shaderProgram, "color");
        h_uLight = glGetUniformLocation(shaderProgram, "light");

        // Retrieve handles to vertex attributes
        h_aPosition = glGetAttribLocation(shaderProgram, "position");
        h_aNormal = glGetAttribLocation(shaderProgram, "normal");
        h_aTexcoord = glGetAttribLocation(shaderProgram, "texcoord");

        // Create 2 textures and load images to them    
        glGenTextures(2, textures);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[0]);

        int width, height;
        unsigned char* image;
        image = SOIL_load_image("Ship_Diffuse.png", &width, &height, 0, SOIL_LOAD_RGB);
        if(image == NULL)
            fprintf(stderr, "NULL pointer.\n");

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        glUniform1i(glGetUniformLocation(shaderProgram, "texKitten"), 0);

        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        SOIL_free_image_data(image);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures[1]);
        image = SOIL_load_image("Ship_Diffuse.png", &width, &height, 0, SOIL_LOAD_RGB);
        if(image == NULL)
            fprintf(stderr, "NULL pointer.\n");

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        glUniform1i(glGetUniformLocation(shaderProgram, "texPuppy"), 1);

        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        SOIL_free_image_data(image);

        /*
        if (!g_Gl2Compatible)
            glBindFragDataLocation(h, 0, "fragColor");
        checkGlErrors();
        */
    }

    void draw(Geometry* geometry) {
        glBindVertexArray(geometry->vao);
        glBindBuffer(GL_ARRAY_BUFFER, geometry->vbo);

        if(geometry->shaderProgram != shaderProgram)
        {
            glEnableVertexAttribArray(h_aPosition);
            glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
            glEnableVertexAttribArray(h_aNormal);
            glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
            glEnableVertexAttribArray(h_aTexcoord);
            glVertexAttribPointer(h_aTexcoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
            
            geometry->shaderProgram = shaderProgram;
        }

        if(geometry->eboLen == 0)
            glDrawArrays(GL_TRIANGLES, 0, geometry->vboLen);
        else
            glDrawElements(GL_TRIANGLES, geometry->eboLen, GL_UNSIGNED_INT, 0);
    }
};

Geometry *g_cube, *g_floor, *g_wall, *g_mesh;
ShaderState *flatShader, *texturedShader;

void draw_scene()
{    
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    Mat4 trans = Mat4::makeTranslation(Vec3(-1, 0, 0));
    trans = transpose(trans);
    //trans = Mat4::makeTranslation(Vec3(-1, 0, 0)) * trans;
    //trans = Mat4::makeZRotation((float)(glfwGetTime()*30));

    //RigTForm tform(Quat::makeZRotation((float)(glfwGetTime()*30)));
    //g_trans = RigTForm(Quat(1, 0, 0, 0));
    //trans = rigTFormToMat(g_trans);
    //trans = transpose(trans)

    glUseProgram(flatShader->shaderProgram);
    glUniformMatrix4fv(flatShader->h_uTrans, 1, GL_FALSE, &(trans[0]));
    glUseProgram(texturedShader->shaderProgram);
    glUniformMatrix4fv(texturedShader->h_uTrans, 1, GL_FALSE, &(trans[0]));


    glUseProgram(flatShader->shaderProgram);
    glUniform3f(flatShader->h_uColor, 0.1f, 0.6f, 0.6f);
    //glUseProgram(texturedShader->shaderProgram);        
    flatShader->draw(g_mesh);

    trans = Mat4::makeTranslation(Vec3(0, 1, -6));
    trans = transpose(trans);
    glUniformMatrix4fv(flatShader->h_uTrans, 1, GL_FALSE, &(trans[0]));
    glUniform3f(flatShader->h_uColor, 1.0f, 1.0f, 1.0f);
    flatShader->draw(g_cube);
    /*
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
        Mat4 view = transpose(rigTFormToMat(g_view));

        glUseProgram(flatShader->shaderProgram);
        glUniformMatrix4fv(flatShader->h_uView, 1, GL_FALSE, &(view[0]));
        glUseProgram(texturedShader->shaderProgram);
        glUniformMatrix4fv(texturedShader->h_uView, 1, GL_FALSE, &(view[0]));
        RigTForm invView = inv(g_view);
        Vec3 trans = invView.getTranslation();
        
        std::cout << "Camera pos: " << trans[0] << " " << trans[1] << " " << trans[2] << "\n";
        draw_scene();
        cursorX = x;
        cursorY = y;
    }
}

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
        movement[0] = -.1f;
        break;
    case GLFW_KEY_D:
        movement[0] = .1f;
        break;
    case GLFW_KEY_W:
        movement[2] = -.1f;
        break;
    case GLFW_KEY_S:
        movement[2] = .1f;
        break;
    default:
        break;
    }
    // had to change from + to - because of view space
    RigTForm m(-movement);
    g_view = m * g_view;
    RigTForm invView =inv(g_view);
    //g_view.setTranslation(g_view.getTranslation() - movement);
    Mat4 view = transpose(rigTFormToMat(g_view));
    
    glUseProgram(flatShader->shaderProgram);
    glUniformMatrix4fv(flatShader->h_uView, 1, GL_FALSE, &(view[0]));
    glUseProgram(texturedShader->shaderProgram);
    glUniformMatrix4fv(texturedShader->h_uView, 1, GL_FALSE, &(view[0]));
    draw_scene();
    Vec3 trans = invView.getTranslation();
    std::cout << "Camera pos: " << trans[0] << " " << trans[1] << " " << trans[2] << "\n";
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

    mesh_verts = readFromObj("galileo.obj", &numVertices);

    
    g_cube = new Geometry(vertices, 36);
    g_mesh = new Geometry(mesh_verts, numVertices);

    g_floor = new Geometry(floor_verts, elements, 4, 6);
    g_wall = new Geometry(wall_verts, elements, 4, 6);

    //readFrom3DS("cube.3ds", &numVertices);
    //readFromObj("Ship.obj", &numVertices);

    //flatShader = new ShaderState(vertexSource, fragmentSource);
    flatShader = new ShaderState(lightVertexSrc, fragmentSource);
    texturedShader = new ShaderState(vertexSource, floorFragSrc);
    //texturedShader = new ShaderState(lightVertexSrc, floorFragSrc);

    glUseProgram(flatShader->shaderProgram);
    
    g_view = RigTForm::lookAt(Vec3(0.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0, 1, 0));
    Vec3 trans = g_view.getTranslation();
    Mat4 view = rigTFormToMat(g_view);
    view = transpose(view);

    glUniformMatrix4fv(flatShader->h_uView, 1, GL_FALSE, &(view[0]));
    g_proj = Mat4::makeProjection(60.0f, 800.0f/600.0f, 0.1f, 30.0f);
    Mat4 proj = transpose(g_proj);
    glUniformMatrix4fv(flatShader->h_uProj, 1, GL_FALSE, &(proj[0]));
    glUniform3f(flatShader->h_uColor, 0.1f, 0.6f, 0.6f);
    glUniform3f(flatShader->h_uLight, 0.0f, 1.0f, -6.0f);
    
    
    // SECOND SHADER
    glUseProgram(texturedShader->shaderProgram);
    glUniformMatrix4fv(texturedShader->h_uView, 1, GL_FALSE, &(view[0]));
    glUniformMatrix4fv(texturedShader->h_uProj, 1, GL_FALSE, &(proj[0]));
    glUniform3f(texturedShader->h_uColor, 0.6f, 0.6f, 0.6f);

    // ---------------------------- RENDERING ------------------------------ //
    
    glEnable(GL_DEPTH_TEST);
    draw_scene();
    while(!glfwWindowShouldClose(window))
    {

        // Apply a rotation        
        // Swap buffers and poll window events
   
        glfwPollEvents();
    }

    // ---------------------------- CLEARING ------------------------------ //

    // Delete allocated resources
    glDeleteProgram(flatShader->shaderProgram);
    glDeleteProgram(texturedShader->shaderProgram);
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
    free(texturedShader);
    free(g_cube);
    free(g_mesh);
    free(g_floor);
    free(g_wall);
    free(mesh_verts);

    // ---------------------------- TERMINATE ----------------------------- //

    // Terminate GLFW
    glfwTerminate();

    return 0;
}
