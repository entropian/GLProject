#ifndef MATERIAL_H
#define MATERIAL_H

#include <GL/glew.h>
#include <GL/glfw3.h>
#include "SOIL.h"
#include "geometry.h"

#define GLSL(src) "#version 150 core\n" #src

extern GLuint textures[2];

struct ShaderState {

    GLuint shaderProgram;

    // Handles to uniforms
    GLint h_uModelViewMat, h_uNormalMat, h_uProjMat, h_uColor, h_uLight;

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
        h_uProjMat = glGetUniformLocation(shaderProgram, "uProjMat");
        h_uModelViewMat = glGetUniformLocation(shaderProgram, "uModelViewMat");
        h_uNormalMat = glGetUniformLocation(shaderProgram, "uNormalMat");
        h_uColor = glGetUniformLocation(shaderProgram, "uColor");
        h_uLight = glGetUniformLocation(shaderProgram, "uLight");

        // Retrieve handles to vertex attributes
        h_aPosition = glGetAttribLocation(shaderProgram, "aPosition");
        h_aNormal = glGetAttribLocation(shaderProgram, "aNormal");
        h_aTexcoord = glGetAttribLocation(shaderProgram, "aTexcoord");

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
/*
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
        GLBINDTEXTURE(GL_TEXTURE_2D, textures[1]);
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


        //if (!g_Gl2Compatible)
            //glBindFragDataLocation(h, 0, "fragColor");
        //checkGlErrors();

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
*/
#endif       
