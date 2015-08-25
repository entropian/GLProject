#ifndef SSAO_H
#define SSAO_H

#include <GL/glew.h>
#include "vec.h"
#include "material.h"
#include "shaders.h"
#include "dfshaders.h"

static const unsigned MAX_SAMPLES = 64;

// SSAO struct
struct SSAOStruct{
    GLuint vao, vbo;
    GLuint noiseTexture;
    GLuint fbo, colorBuffer;
    GLuint blurFbo, colorBufferBlur;
    GLuint firstPassProgram, blurPassProgram;
};

void deleteSSAO(SSAOStruct &ssaos)
{
    glDeleteVertexArrays(1, &(ssaos.vao));
    glDeleteBuffers(1, &(ssaos.vbo));
    glDeleteTextures(1, &(ssaos.noiseTexture));
    glDeleteTextures(1, &(ssaos.colorBuffer));
    glDeleteTextures(1, &(ssaos.colorBufferBlur));
    glDeleteFramebuffers(1, &(ssaos.fbo));
    glDeleteFramebuffers(1, &(ssaos.blurFbo));
    glDeleteProgram(ssaos.firstPassProgram);
    glDeleteProgram(ssaos.blurPassProgram);    
}

float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

void initSSAO(SSAOStruct &ssaos, int windowWidth, int windowHeight, int numSamples, const float radius)
{
    if(numSamples > MAX_SAMPLES)
    {
        fprintf(stderr, "Max SSAO samples is %d.\n", MAX_SAMPLES);
        numSamples = MAX_SAMPLES;
    }

    // Shaders
    ssaos.firstPassProgram = compileAndLinkShaders(RTBVertSrc, SSAOFragSrc);
    glUseProgram(ssaos.firstPassProgram);
    glUniform1i(glGetUniformLocation(ssaos.firstPassProgram, "gPositionDepth"), 0);
    glUniform1i(glGetUniformLocation(ssaos.firstPassProgram, "gNormalSpec"), 1);
    glUniform1i(glGetUniformLocation(ssaos.firstPassProgram, "texNoise"), 2);
    glUniform1i(glGetUniformLocation(ssaos.firstPassProgram, "kernelSize"), numSamples);
    glUniform1f(glGetUniformLocation(ssaos.firstPassProgram, "radius"), radius);    

    ssaos.blurPassProgram = compileAndLinkShaders(RTBVertSrc, SSAOBlurFragSrc);
        glUseProgram(ssaos.blurPassProgram);
    glUniform1i(glGetUniformLocation(ssaos.firstPassProgram, "ssao"), 0);

    // Generate samples
    glUseProgram(ssaos.firstPassProgram);
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
    std::default_random_engine generator;
    for(int i = 0; i < numSamples; i++)
    {
        Vec3 sample(randomFloats(generator) * 2.0 - 1.0,
                    randomFloats(generator) * 2.0 - 1.0,
                    randomFloats(generator));
        sample = normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / (float)numSamples;
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        glUniform3fv(glGetUniformLocation(ssaos.firstPassProgram, ("samples[" + std::to_string(i) + "]").c_str()),
                     1, &(sample[0]));
    }

    // Generate rotational noise
    Vec3 noises[16];
    for(int i = 0; i < 16; i++)
    {

        Vec3 noise(randomFloats(generator) * 2.0 - 1.0,
                   randomFloats(generator) * 2.0 - 1.0,
                   0.0f);
        noises[i] = noise;
    }

    // Noise texture
    glGenTextures(1, &(ssaos.noiseTexture));
    glBindTexture(GL_TEXTURE_2D, ssaos.noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &(noises[0]));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);    

    // SSAO framebuffer 
    glGenFramebuffers(1, &(ssaos.fbo));
    glBindFramebuffer(GL_FRAMEBUFFER, ssaos.fbo);
    glGenTextures(1, &(ssaos.colorBuffer));
    glBindTexture(GL_TEXTURE_2D, ssaos.colorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaos.colorBuffer, 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "SSAO Framebuffer not complete!\n");

    // Blur framebuffer
    glGenFramebuffers(1, &(ssaos.blurFbo));
    glBindFramebuffer(GL_FRAMEBUFFER, ssaos.blurFbo);
    glGenTextures(1, &(ssaos.colorBufferBlur));
    glBindTexture(GL_TEXTURE_2D, ssaos.colorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaos.colorBufferBlur, 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "SSAO Blur Framebuffer not complete!\n");    

    // The quad that covers the whole viewport
    GLfloat vertices[] = {
        -1.0f,  -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0, -1.0, 1.0f, 0.0f,
        -1.0, 1.0, 0.0f, 1.0f,
        1.0, 1.0, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &(ssaos.vao));
    glBindVertexArray(ssaos.vao);

    glGenBuffers(1, &(ssaos.vbo));
    glBindBuffer(GL_ARRAY_BUFFER, ssaos.vbo);
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
