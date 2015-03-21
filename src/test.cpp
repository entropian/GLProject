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

GLint uniView;

static double cursorX;
static double cursorY;

static RigTForm g_view;

void draw_scene()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
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

        g_view = RigTForm(Quat::makeYRotation(dx)) * g_view;
        g_view = RigTForm(Quat::makeXRotation(dy)) * g_view;
        Mat4 view = rigTFormToMat(g_view);
        view = transpose(view);

        glUniformMatrix4fv(uniView, 1, GL_FALSE, &(view[0]));
        cursorX = x;
        cursorY = y;        
    }
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

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);

    // Init GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

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
    Mat4 moveCube1 = Mat4::makeTranslation(Vec3(0.7f, 0, 0));


    RigTForm moveCube(Vec3(0.0f, 0, 0));

    GLuint elements[] = {
         0, 1, 2,
         2, 3, 0
    };

    // Create and compile the vertex shader
    const char* vertexSource = GLSL(
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

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    GLint status;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);

    if(status != GL_TRUE)
        fprintf(stderr, "Vertex shader compiled incorrectly.\n");

    // Create and compile the fragment shader
    const char* fragmentSource = GLSL(
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

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);

    if(status != GL_TRUE)
        fprintf(stderr, "Fragment shader compiled incorrectly.\n");

    // Link the vertex and fragment shader into a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    // Transform matrices
    GLint uniTrans = glGetUniformLocation(shaderProgram, "trans");

    //Mat4 view = Mat4::lookAt(Vec3(1.2f, 1.2f, 1.2f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f));
    g_view = RigTForm::lookAt(Vec3(1.2f, 1.2f, 1.2f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f));
    Mat4 view = rigTFormToMat(g_view);
    view = transpose(view);

    uniView = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(uniView, 1, GL_FALSE, &(view[0]));

    Mat4 proj = Mat4::makeProjection(80.0f, 800.0f/600.0f, 0.1f, 10.0f);
    proj = transpose(proj);

    GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, &(proj[0]));

    // Create 2 textures and load images to them
    GLuint textures[2];
    glGenTextures(2, textures);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    
    int width, height;
    unsigned char* image;
    image = SOIL_load_image("sample.png", &width, &height, 0, SOIL_LOAD_RGB);
    if(image == NULL)
        fprintf(stderr, "NULL pointer.\n");

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glUniform1i(glGetUniformLocation(shaderProgram, "texKitten"), 0);

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
    glUniform1i(glGetUniformLocation(shaderProgram, "texPuppy"), 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SOIL_free_image_data(image);

    // Create Vertex Array Object
    GLuint vao1, vao2;
    GLuint vbo1, vbo2;
    GLuint ebo;
    glGenVertexArrays(1, &vao1);
    glGenVertexArrays(1, &vao2);
    glGenBuffers(1, &vbo1);
    glGenBuffers(1, &vbo2);
    glGenBuffers(1, &ebo);

    
    glBindVertexArray(vao1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements,
        GL_STATIC_DRAW);

    
    // Specify the layout of the vertex data
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);


    GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
    glEnableVertexAttribArray(colAttrib);
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));


    GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    /*
    GLint uColor = glGetUniformLocation(shaderProgram, "uColor");
    Vec3 colorVec(1, 0, 0);
    glUniform3fv(uColor, 1, &(colorVec[0]));
    */

    /*
    glBindVertexArray(vao2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), verts2, GL_STATIC_DRAW);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    
    // Specify the layout of the vertex data
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);

    glEnableVertexAttribArray(colAttrib);
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glBindVertexArray(0);
    */

    // ---------------------------- RENDERING ------------------------------ //
    glEnable(GL_DEPTH_TEST);
    while(!glfwWindowShouldClose(window))
    {
		// Clear the screen to black
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Apply a rotation

        Mat4 trans;
        //trans = Mat4::makeZRotation((float)(glfwGetTime()*30));

        //RigTForm tform(Quat::makeZRotation((float)(glfwGetTime()*30)));
        RigTForm tform(Quat(1, 0, 0, 0));
        //tform = tform*moveCube;
        trans = rigTFormToMat(tform);
        trans = transpose(trans);

        glUniformMatrix4fv(uniTrans, 1, GL_FALSE, &(trans[0]));

        
        // Draw a quad from the 2 triangles using 6 elements
        glBindVertexArray(vao1);
        //glBindBuffer(GL_ARRAY_BUFFER, vbo1);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        /*
        glBindVertexArray(vao2);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        */

        // Swap buffers and poll window events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ---------------------------- CLEARING ------------------------------ //

    // Delete allocated resources
    glDeleteProgram(shaderProgram);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteTextures(2, textures);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo1);
    glDeleteVertexArrays(1, &vao1);
    glDeleteVertexArrays(1, &vao2);

    // ---------------------------- TERMINATE ----------------------------- //

    // Terminate GLFW
    glfwTerminate();

    return 0;
}
