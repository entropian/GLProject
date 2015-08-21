#ifndef DEFERRED_H
#define DEFERRED_H

#include <GL/glew.h>
#include "material.h"
#include "shaders.h"
#include "dfshaders.h"

// Deferred rendering struct
struct DFStruct{
    GLuint gBuffer, gPositionDepth, gNormalSpec, gDiffuse;
    GLuint vao, vbo, rboDepth;
    GLuint shaderProgram;
    GLuint defaultShader, shadowShader;
    GLuint showSSAO, showNormal, showDiffuse, showSpecular;
};

void deleteDeferredStruct(DFStruct &df)
{
    glDeleteVertexArrays(1, &(df.vao));
    glDeleteBuffers(1, &(df.vbo));
    glDeleteTextures(1, &(df.gPositionDepth));
    glDeleteTextures(1, &(df.gNormalSpec));
    glDeleteTextures(1, &(df.gDiffuse));
    glDeleteFramebuffers(1, &(df.gBuffer));
    glDeleteRenderbuffers(1, &(df.rboDepth));
    //glDeleteProgram(df.shaderProgram);
    glDeleteProgram(df.defaultShader);
    glDeleteProgram(df.shadowShader);    
    glDeleteProgram(df.showSSAO);
    glDeleteProgram(df.showNormal);
    glDeleteProgram(df.showDiffuse);
    glDeleteProgram(df.showSpecular);
}

void initDeferredRender(DFStruct &df, int windowWidth, int windowHeight)
{
    df.defaultShader = compileAndLinkShaders(RTBVertSrc, LightPassFragSrc);
    df.shaderProgram = df.defaultShader;
    glUseProgram(df.shaderProgram);
    glUniform1i(glGetUniformLocation(df.shaderProgram, "gPositionDepth"), 0);
    glUniform1i(glGetUniformLocation(df.shaderProgram, "gNormalSpec"), 1);
    glUniform1i(glGetUniformLocation(df.shaderProgram, "gDiffuse"), 2);
    glUniform1i(glGetUniformLocation(df.shaderProgram, "shadowMap"), 3);
    glUniform1i(glGetUniformLocation(df.shaderProgram, "ssao"), 4);

    df.shadowShader = compileAndLinkShaders(RTBVertSrc, shLightPassFragSrc);
    df.shaderProgram = df.shadowShader;
    glUseProgram(df.shaderProgram);
    glUniform1i(glGetUniformLocation(df.shaderProgram, "gPositionDepth"), 0);
    glUniform1i(glGetUniformLocation(df.shaderProgram, "gNormalSpec"), 1);
    glUniform1i(glGetUniformLocation(df.shaderProgram, "gDiffuse"), 2);
    glUniform1i(glGetUniformLocation(df.shaderProgram, "shadowMap"), 3);
    glUniform1i(glGetUniformLocation(df.shaderProgram, "ssao"), 4);    

    df.showSSAO = compileAndLinkShaders(RTBVertSrc, showSSAOFragSrc);
    df.shaderProgram = df.showSSAO;
    glUseProgram(df.shaderProgram);
    glUniform1i(glGetUniformLocation(df.shaderProgram, "ssao"), 4);

    df.showNormal = compileAndLinkShaders(RTBVertSrc, showNormalFragSrc);
    df.shaderProgram = df.showNormal;
    glUseProgram(df.shaderProgram);
    glUniform1i(glGetUniformLocation(df.shaderProgram, "gNormalSpec"), 1);

    df.showDiffuse = compileAndLinkShaders(RTBVertSrc, showDiffuseFragSrc);
    df.shaderProgram = df.showDiffuse;
    glUseProgram(df.shaderProgram);    
    glUniform1i(glGetUniformLocation(df.shaderProgram, "gDiffuse"), 2);

    df.showSpecular = compileAndLinkShaders(RTBVertSrc, showSpecularFragSrc);
    df.shaderProgram = df.showSpecular;
    glUseProgram(df.shaderProgram);
    glUniform1i(glGetUniformLocation(df.shaderProgram, "gDiffuse"), 2);
    
    df.shaderProgram = df.defaultShader;
    
    glGenFramebuffers(1, &(df.gBuffer));
    glBindFramebuffer(GL_FRAMEBUFFER, df.gBuffer);

    glGenTextures(1, &(df.gPositionDepth));
    glBindTexture(GL_TEXTURE_2D, df.gPositionDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, df.gPositionDepth, 0);

    glGenTextures(1, &(df.gNormalSpec));
    glBindTexture(GL_TEXTURE_2D, df.gNormalSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, df.gNormalSpec, 0);
    
    glGenTextures(1, &(df.gDiffuse));
    glBindTexture(GL_TEXTURE_2D, df.gDiffuse);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, df.gDiffuse, 0);

    GLuint attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments);

    glGenRenderbuffers(1, &(df.rboDepth));
    glBindRenderbuffer(GL_RENDERBUFFER, df.rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, df.rboDepth);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "Framebuffer not complete!\n");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);    

    // The quad that covers the whole viewport
    GLfloat vertices[] = {
        -1.0f,  -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0, -1.0, 1.0f, 0.0f,
        -1.0, 1.0, 0.0f, 1.0f,
        1.0, 1.0, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &(df.vao));
    glBindVertexArray(df.vao);

    glGenBuffers(1, &(df.vbo));
    glBindBuffer(GL_ARRAY_BUFFER, df.vbo);
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

#endif
