#include <stdio.h>
#include <GL/glew.h>
#include <GL/glfw3.h>
#include "SOIL.h"
#include <iostream>

#include "vec.h"
#include "mat.h"
#include "quat.h"
#include "rigtform.h"


#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"


#define GLSL(src) "#version 150 core\n" #src

GLint uniView1, uniView2;

static double cursorX;
static double cursorY;

static RigTForm g_view, g_trans;
static Mat4 g_proj;

GLuint textures[2];
GLuint shaderProgram1, shaderProgram2;
GLuint vao1, vao2;
GLuint vbo1, vbo2, ebo;
GLint uniTrans1, uniTrans2;

GLFWwindow* window;

//up vector

// Create and compile the vertex shader
static const char* vertexSource = GLSL(
    uniform mat4 trans;
    uniform mat4 view;
    uniform mat4 proj;

    in vec3 position;
    in vec3 color;
    in vec2 texcoord;

    out vec3 Color;
    out vec2 Texcoord;
        
    void main() {
        Color = color;
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

void draw_scene()
{    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    Mat4 trans = Mat4::makeTranslation(Vec3(-1, 0, 0));
    trans = transpose(trans);
    //trans = Mat4::makeTranslation(Vec3(-1, 0, 0)) * trans;
    //trans = Mat4::makeZRotation((float)(glfwGetTime()*30));

    //RigTForm tform(Quat::makeZRotation((float)(glfwGetTime()*30)));
    //g_trans = RigTForm(Quat(1, 0, 0, 0));
    //trans = rigTFormToMat(g_trans);
    //trans = transpose(trans)

    glUseProgram(shaderProgram1);
    glUniformMatrix4fv(uniTrans1, 1, GL_FALSE, &(trans[0]));
    glUseProgram(shaderProgram2);
    glUniformMatrix4fv(uniTrans2, 1, GL_FALSE, &(trans[0]));

    glUseProgram(shaderProgram1);
    // Draw a quad from the 2 triangles using 6 elements
    glBindVertexArray(vao1);
    //glBindBuffer(GL_ARRAY_BUFFER, vbo1);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glUseProgram(shaderProgram2);
    glBindVertexArray(vao2);
    //glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    /*
      glBindVertexArray(vao2);
      glDrawArrays(GL_TRIANGLES, 0, 36);
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

        glUseProgram(shaderProgram1);
        glUniformMatrix4fv(uniView1, 1, GL_FALSE, &(view[0]));
        glUseProgram(shaderProgram2);
        glUniformMatrix4fv(uniView2, 1, GL_FALSE, &(view[0]));
        //Vec3 trans = g_view.getTranslation();
        //std::cout << "Camera pos: " << trans[0] << " " << trans[1] << " " << trans[2] << "\n";
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
    g_view.setTranslation(g_view.getTranslation() - movement);
    Mat4 view = transpose(rigTFormToMat(g_view));

    glUseProgram(shaderProgram1);
    glUniformMatrix4fv(uniView1, 1, GL_FALSE, &(view[0]));
    glUseProgram(shaderProgram2);
    glUniformMatrix4fv(uniView2, 1, GL_FALSE, &(view[0]));
    draw_scene();
}

void initTextures()
{
    // Create 2 textures and load images to them    
    glGenTextures(2, textures);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);

    glUseProgram(shaderProgram2);
    int width, height;
    unsigned char* image;
    image = SOIL_load_image("sample.png", &width, &height, 0, SOIL_LOAD_RGB);
    if(image == NULL)
        fprintf(stderr, "NULL pointer.\n");

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glUniform1i(glGetUniformLocation(shaderProgram2, "texKitten"), 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SOIL_free_image_data(image);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    image = SOIL_load_image("sample2.png", &width, &height, 0, SOIL_LOAD_RGB);
    if(image == NULL)
        fprintf(stderr, "NULL pointer.\n");

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glUniform1i(glGetUniformLocation(shaderProgram2, "texPuppy"), 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SOIL_free_image_data(image);
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
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,

        // back
        -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,

        // left
        -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,

        // right
        0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

        // bottom
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,

        // top
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
    };

    GLfloat floor_verts[] = {
        -2.0f, -0.5f, -2.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        2.0f, -0.5f, -2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        2.0f, -0.5f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        -2.0f, -0.5f, 2.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    };

    GLuint elements[] = {
        0, 1, 2,
        0, 2, 3
    };
    
    // Create Vertex Array Object
    glGenVertexArrays(1, &vao1);
    glGenVertexArrays(1, &vao2);
    glGenBuffers(1, &vbo1);
    glGenBuffers(1, &vbo2);
    glGenBuffers(1, &ebo);

    
    glBindVertexArray(vao1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(vao2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_verts), floor_verts, GL_STATIC_DRAW);

    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    glBindVertexArray(vao1);
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    GLint status;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);

    if(status != GL_TRUE)
        fprintf(stderr, "Vertex shader compiled incorrectly.\n");
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);

    if(status != GL_TRUE)
        fprintf(stderr, "Fragment shader compiled incorrectly.\n");

    // Link the vertex and fragment shader into a shader program
    shaderProgram1 = glCreateProgram();
    glAttachShader(shaderProgram1, vertexShader);
    glAttachShader(shaderProgram1, fragmentShader);
    glBindFragDataLocation(shaderProgram1, 0, "outColor");
    glLinkProgram(shaderProgram1);
    glUseProgram(shaderProgram1);


    // Transform matrices
    uniTrans1 = glGetUniformLocation(shaderProgram1, "trans");

    g_view = RigTForm::lookAt(Vec3(0.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0, 1, 0));
    Vec3 trans = g_view.getTranslation();
    std::cout << "Camera pos: " << trans[0] << " " << trans[1] << " " << trans[2] << "\n";
    Mat4 view = rigTFormToMat(g_view);
    view = transpose(view);


    uniView1 = glGetUniformLocation(shaderProgram1, "view");
    glUniformMatrix4fv(uniView1, 1, GL_FALSE, &(view[0]));

    g_proj = Mat4::makeProjection(60.0f, 800.0f/600.0f, 0.1f, 10.0f);
    Mat4 proj = transpose(g_proj);

    GLint uniProj1 = glGetUniformLocation(shaderProgram1, "proj");
    glUniformMatrix4fv(uniProj1, 1, GL_FALSE, &(proj[0]));

    glBindBuffer(GL_ARRAY_BUFFER, vbo1);
    // Specify the layout of the vertex data
    GLint posAttrib1 = glGetAttribLocation(shaderProgram1, "position");
    glEnableVertexAttribArray(posAttrib1);
    glVertexAttribPointer(posAttrib1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);


    GLint colAttrib1 = glGetAttribLocation(shaderProgram1, "color");
    glEnableVertexAttribArray(colAttrib1);
    glVertexAttribPointer(colAttrib1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));


    GLint texAttrib1 = glGetAttribLocation(shaderProgram1, "texcoord");
    glEnableVertexAttribArray(texAttrib1);
    glVertexAttribPointer(texAttrib1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glBindVertexArray(vao1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glEnableVertexAttribArray(posAttrib1);
    glVertexAttribPointer(posAttrib1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);

    glEnableVertexAttribArray(colAttrib1);
    glVertexAttribPointer(colAttrib1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(texAttrib1);
    glVertexAttribPointer(texAttrib1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    ////////////////////////////////////////////////////////////////
    // SECOND SHADER
    glBindVertexArray(vao1);
    GLuint floorFragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(floorFragShader, 1, &floorFragSrc, NULL);
    glCompileShader(floorFragShader);

    glGetShaderiv(floorFragShader, GL_COMPILE_STATUS, &status);

    if(status != GL_TRUE)
        fprintf(stderr, "Fragment shader compiled incorrectly.\n");

    // Link the vertex and fragment shader into a shader program
    shaderProgram2 = glCreateProgram();
    glAttachShader(shaderProgram2, vertexShader);
    glAttachShader(shaderProgram2, floorFragShader);
    glBindFragDataLocation(shaderProgram2, 0, "outColor");
    glLinkProgram(shaderProgram2);
    glUseProgram(shaderProgram2);

    // Transform matrices
    uniTrans2 = glGetUniformLocation(shaderProgram2, "trans");

    g_view = RigTForm::lookAt(Vec3(0.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0, 1, 0));
    trans = g_view.getTranslation();
    std::cout << "Camera pos: " << trans[0] << " " << trans[1] << " " << trans[2] << "\n";
    view = rigTFormToMat(g_view);
    view = transpose(view);


    uniView2 = glGetUniformLocation(shaderProgram2, "view");
    glUniformMatrix4fv(uniView2, 1, GL_FALSE, &(view[0]));

    g_proj = Mat4::makeProjection(60.0f, 800.0f/600.0f, 0.1f, 10.0f);
    proj = transpose(g_proj);

    GLint uniProj2 = glGetUniformLocation(shaderProgram2, "proj");
    glUniformMatrix4fv(uniProj2, 1, GL_FALSE, &(proj[0]));

    glBindBuffer(GL_ARRAY_BUFFER, vbo1);
    // Specify the layout of the vertex data
    GLint posAttrib2 = glGetAttribLocation(shaderProgram2, "position");
    glEnableVertexAttribArray(posAttrib2);
    glVertexAttribPointer(posAttrib2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);


    GLint colAttrib2 = glGetAttribLocation(shaderProgram2, "color");
    glEnableVertexAttribArray(colAttrib2);
    glVertexAttribPointer(colAttrib2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));


    GLint texAttrib2 = glGetAttribLocation(shaderProgram2, "texcoord");
    glEnableVertexAttribArray(texAttrib2);
    glVertexAttribPointer(texAttrib2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glBindVertexArray(vao2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glEnableVertexAttribArray(posAttrib2);
    glVertexAttribPointer(posAttrib2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);

    glEnableVertexAttribArray(colAttrib2);
    glVertexAttribPointer(colAttrib2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(texAttrib2);
    glVertexAttribPointer(texAttrib2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    

    glBindBuffer(GL_ARRAY_BUFFER, vbo1);
    initTextures();
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
    glDeleteProgram(shaderProgram1);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteTextures(2, textures);
    glDeleteBuffers(1, &vbo1);
    glDeleteVertexArrays(1, &vao1);

    // ---------------------------- TERMINATE ----------------------------- //

    // Terminate GLFW
    glfwTerminate();

    return 0;
}
