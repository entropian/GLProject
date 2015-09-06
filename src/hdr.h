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
    GLuint shaderProgram, toneMap, toneMapBloom, showBloom;
};

void deleteHDRStruct(HDRStruct &hdr)
{
    glDeleteVertexArrays(1, &(hdr.vao));
    glDeleteBuffers(1, &(hdr.vbo));
    glDeleteTextures(1, &(hdr.colorBuffer));
    glDeleteTextures(1, &(hdr.brightColorBuffer));    
    glDeleteFramebuffers(1, &(hdr.fbo));
    //glDeleteProgram(hdr.shaderProgram);
    glDeleteProgram(hdr.toneMap);    
    glDeleteProgram(hdr.showBloom);
}

void initHDR(HDRStruct &hdr, const int windowWidth, const int windowHeight, const bool bloom)
{
    hdr.showBloom = compileAndLinkShaders(RTBVertSrc, showBloomFragSrc);
    glUseProgram(hdr.showBloom);
    glUniform1i(glGetUniformLocation(hdr.showBloom, "bloomBlurBuffer"), 1);
    glUniform1f(glGetUniformLocation(hdr.showBloom, "exposure"), 2.0f);    
    
    //hdr.toneMap = compileAndLinkShaders(RTBVertSrc, bloom ? bloomFragSrc : exposureHDRFragSrc);
    hdr.toneMap = compileAndLinkShaders(RTBVertSrc, exposureHDRFragSrc);    
    glUseProgram(hdr.toneMap);
    glUniform1i(glGetUniformLocation(hdr.toneMap, "hdrBuffer"), 0);
    glUniform1f(glGetUniformLocation(hdr.toneMap, "exposure"), 2.0f);

    hdr.toneMapBloom = compileAndLinkShaders(RTBVertSrc, bloomFragSrc);    
    glUseProgram(hdr.toneMapBloom);
    glUniform1i(glGetUniformLocation(hdr.toneMapBloom, "hdrBuffer"), 0);
    glUniform1i(glGetUniformLocation(hdr.toneMapBloom, "bloomBlurBuffer"), 1);        
    glUniform1f(glGetUniformLocation(hdr.toneMapBloom, "exposure"), 2.0f);    

    hdr.shaderProgram = hdr.toneMapBloom;

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
    // Setting it to clamp to edge to stop bloom blurring from sampling the opposite edge of the texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, hdr.brightColorBuffer, 0);

    GLuint attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "HDR Framebuffer not complete!\n");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    initScreenQuad(&(hdr.vao), &(hdr.vbo));
}

#endif
