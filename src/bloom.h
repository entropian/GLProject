#ifndef BLOOM_H
#define BLOOM_H

#include <GL/glew.h>
#include "material.h"
#include "screenquad.h"

struct BloomBlurStruct
{
    GLuint vao, vbo;
    GLuint pingpongFBO[2];
    GLuint pingpongBuffer[2];
    GLuint shaderProgram;
};

void deleteBloomBlur(BloomBlurStruct &bb)
{
    glDeleteVertexArrays(1, &(bb.vao));
    glDeleteBuffers(1, &(bb.vbo));
    glDeleteTextures(2, bb.pingpongBuffer);
    glDeleteFramebuffers(2, bb.pingpongFBO);
    glDeleteProgram(bb.shaderProgram);
}

void initBloomBlur(BloomBlurStruct &bb, int windowWidth, int windowHeight)
{
    bb.shaderProgram = compileAndLinkShaders(RTBVertSrc, BloomBlurFragSrc);
    glUseProgram(bb.shaderProgram);
    glUniform1i(glGetUniformLocation(bb.shaderProgram, "brightColorBuffer"), 0);

    glGenFramebuffers(2, bb.pingpongFBO);
    glGenTextures(2, bb.pingpongBuffer);
    for(GLuint i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, bb.pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, bb.pingpongBuffer[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bb.pingpongBuffer[i], 0);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            fprintf(stderr, "HDR Framebuffer not complete!\n");    
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    initScreenQuad(&(bb.vao), &(bb.vbo));
}

#endif
