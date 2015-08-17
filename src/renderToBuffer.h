#ifndef RTB_H
#define RTB_H

#include <GL/glew.h>

#if __GNUG__
#   include "SOIL/SOIL.h"
#else
#   include "SOIL.h"
#endif

#include "material.h"
#include "shaders.h"


// struct for Render-to-buffer 
struct RTB{
    GLuint framebuffer, texColorBuffer, rbo;
    GLuint vao, vbo;
    GLuint shaderProgram;
    GLuint texture;
};


// Depth map struct
struct DepthMap{
    GLuint depthMapFBO;
    GLuint depthMap;
    GLsizei SHADOW_WIDTH, SHADOW_HEIGHT;
    Material *depthMapMaterial;
};

//extern DepthMap g_depthMap;


void initRenderToBuffer(RTB &rtb, int windowWidth, int windowHeight)
{
    rtb.shaderProgram = compileAndLinkShaders(RTBVertSrc, RTBFragSrc);
    glUseProgram(rtb.shaderProgram);
    // The quad that covers the whole viewport
    GLfloat vertices[] = {
        -1.0f,  -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0, -1.0, 1.0f, 0.0f,
        -1.0, 1.0, 0.0f, 1.0f,
        1.0, 1.0, 1.0f, 1.0f
    };    

    // Generate and bind buffer objects
    glGenVertexArrays(1, &(rtb.vao));
    glBindVertexArray(rtb.vao);

    glGenBuffers(1, &(rtb.vbo));
    glBindBuffer(GL_ARRAY_BUFFER, rtb.vbo);
    //sizeof works because vertices is an array, not a pointer.
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Setup vertex attributes
    //GLint posAttrib = glGetAttribLocation(RTBProgram, "aPosition");
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

    // Setup texture handle
    rtb.texture = glGetUniformLocation(rtb.shaderProgram, "screenTexture");    

    // Generate framebuffer
    glGenFramebuffers(1, &(rtb.framebuffer));
    glBindFramebuffer(GL_FRAMEBUFFER, rtb.framebuffer);

    // Generate texture
    glGenTextures(1, &(rtb.texColorBuffer));
    glBindTexture(GL_TEXTURE_2D, rtb.texColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);    
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach texture to currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rtb.texColorBuffer, 0);

    // Create renderbuffer object for stencil and depth buffer
    glGenRenderbuffers(1, &(rtb.rbo));
    glBindRenderbuffer(GL_RENDERBUFFER, rtb.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rtb.rbo);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "Frame buffer is not complete.\n");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void drawBufferToScreen(RTB &rtb)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);                
    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glUseProgram(rtb.shaderProgram);
    glBindVertexArray(rtb.vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rtb.texColorBuffer);
    //glBindTexture(GL_TEXTURE_2D, g_depthMap.depthMap);
    glUniform1i(rtb.texture, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

#endif
