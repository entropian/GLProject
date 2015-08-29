#ifndef QUAD_H
#define QUAD_H

#include <GL/glew.h>

void initScreenQuad(GLuint *vao, GLuint *vbo)
{
    // The quad that covers the whole viewport
    GLfloat vertices[] = {
        -1.0f,  -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0, -1.0, 1.0f, 0.0f,
        -1.0, 1.0, 0.0f, 1.0f,
        1.0, 1.0, 1.0f, 1.0f
    };

    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);

    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Setup vertex attributes
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    // Texcoord
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glBindVertexArray(0);    
}

void drawScreenQuad(const GLuint framebuffer, const GLuint shaderProgram, const GLuint vao, const GLuint colorBuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glDisable(GL_DEPTH_TEST);
    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);    
}

void drawScreenQuadMultiTex(const GLuint framebuffer, const GLuint shaderProgram, const GLuint vao, const GLuint colorBuffers[],
                            const GLuint numColorBuffers)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glDisable(GL_DEPTH_TEST);
    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    for(GLuint i = 0; i < numColorBuffers; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
    }
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);        
}

#endif
