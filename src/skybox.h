#ifndef SKYBOX_H
#define SKYBOX_H

#include <GL/glew.h>
#include "mat.h"

struct Skybox{
    GLuint cubemap, cubemapUniformHandle, viewMatHandle;
    GLuint vao, vbo, shaderProgram;    
};


// Loads texture files specified by faces and returns handle to the resulting cubemap
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

    // Texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    return textureID;
}

void initSkybox(Skybox &skybox)
{
    GLfloat vertices[] = {
        -1, -1, -1, -1, 1, -1, 1, 1, -1,    // front
        -1, -1, -1, 1, 1, -1, 1, -1, -1,
        -1, -1, 1, -1, 1, 1, 1, 1, 1,       // back
        -1, 1, 1, 1, 1, 1, 1, -1, 1,
        -1, -1, 1, -1, 1, 1, -1, 1, -1,     // left
        -1, -1, 1, -1, 1, -1, -1, -1, -1,
        1, -1, 1, 1, 1, -1, 1, -1, -1,      // right
        1, -1, 1, 1, 1, -1, 1, -1, -1,
        -1, 1, -1, -1, 1, 1, 1, 1, 1,       // top
        -1, 1, -1, 1, 1, 1, 1, 1, -1,
        -1, -1, -1, -1, -1, 1, 1, -1, 1,    // bottom
        -1, -1, -1, 1, -1, 1, 1, -1, -1        
    };
    
    skybox.shaderProgram = compileAndLinkShaders(skyboxVertSrc, skyboxFragSrc);
    glUseProgram(skybox.shaderProgram);
    
    Mesh cubeMesh;
    cubeMesh.loadOBJFile("cube.obj");
    Geometry *cube = cubeMesh.produceGeometry(PNX);    

    glGenVertexArrays(1, &(skybox.vao));
    glBindVertexArray(skybox.vao);   
    skybox.vbo = cube->vbo;
    glBindBuffer(GL_ARRAY_BUFFER, skybox.vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);

    const char *skyboxTextureFiles[6] = {"skybox_right.jpg", "skybox_left.jpg",
                                "skybox_top.jpg", "skybox_bottom.jpg", "skybox_back.jpg", "skybox_front.jpg"};        
    skybox.cubemap = loadCubemap(skyboxTextureFiles);
    
    GLuint blockIndex = glGetUniformBlockIndex(skybox.shaderProgram, "UniformBlock");
    glUniformBlockBinding(skybox.shaderProgram, blockIndex, 0);    
    skybox.cubemapUniformHandle = glGetUniformLocation(skybox.shaderProgram, "skybox");
    skybox.viewMatHandle = glGetUniformLocation(skybox.shaderProgram, "uViewMat");
    glBindVertexArray(0);
    glUseProgram(0);
}

void drawSkybox(Skybox &skybox, Mat4 &viewRotationMat)
{
    glDepthMask(GL_FALSE);
    glUseProgram(skybox.shaderProgram);    
    glUniformMatrix4fv(skybox.viewMatHandle, 1, GL_TRUE, &(viewRotationMat[0]));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.cubemap);
    glUniform1i(skybox.cubemapUniformHandle, 0);
    glBindVertexArray(skybox.vao);
    glBindBuffer(GL_ARRAY_BUFFER, skybox.vbo);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);        
    glDepthMask(GL_TRUE);  
}

#endif
