#ifndef HDR_H
#define HDR_H

#include <GL/glew.h>
#include "shaders.h"
#include "dfshaders.h"
#include "screenquad.h"

struct HDRStruct
{
    GLuint vao, vbo;
    GLuint fbo, colorBuffer, brightColorBuffer;
    GLuint shaderProgram;
};

void deleteHDRStruct(HDRStruct &hdr)
{
    glDeleteVertexArrays(1, &(hdr.vao));
    glDeleteBuffers(1, &(hdr.vbo));
    glDeleteTextures(1, &(hdr.colorBuffer));
    glDeleteTextures(1, &(hdr.brightColorBuffer));    
    glDeleteFramebuffers(1, &(hdr.fbo));
    glDeleteProgram(hdr.shaderProgram);
}

void initHDR(HDRStruct &hdr, const int windowWidth, const int windowHeight, const bool bloom)
{
    hdr.shaderProgram = compileAndLinkShaders(RTBVertSrc, bloom ? bloomFragSrc : exposureHDRFragSrc);
    glUseProgram(hdr.shaderProgram);
    glUniform1i(glGetUniformLocation(hdr.shaderProgram, "hdrBuffer"), 0);
    if(bloom)
        glUniform1i(glGetUniformLocation(hdr.shaderProgram, "bloomBlurBuffer"), 1);        
    glUniform1f(glGetUniformLocation(hdr.shaderProgram, "exposure"), 2.0f);

    glGenFramebuffers(1, &(hdr.fbo));
    glBindFramebuffer(GL_FRAMEBUFFER, hdr.fbo);
    
    glGenTextures(1, &(hdr.colorBuffer));
    glBindTexture(GL_TEXTURE_2D, hdr.colorBuffer);
    // GL_RGBA16F for HDR    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdr.colorBuffer, 0);

    glGenTextures(1, &(hdr.brightColorBuffer));
    glBindTexture(GL_TEXTURE_2D, hdr.brightColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, hdr.brightColorBuffer, 0);

    GLuint attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "HDR Framebuffer not complete!\n");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    initScreenQuad(&(hdr.vao), &(hdr.vbo));
}

#endif
