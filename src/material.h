#ifndef MATERIAL_H
#define MATERIAL_H

#include <GL/glew.h>
#include <GL/glfw3.h>
#include "SOIL.h"
#include "geometry.h"

bool g_debugUniformString = false;

struct Uniform
{
    char name[20];
    Vec3 vec;
    RigTForm rbt;
    GLenum type;
};

struct UniformDesc
{
    char name[20];
    GLuint index;
    GLenum type;
    GLuint handle;
    GLsizei size;
};

static void readAndCompileShaders(const char *vs, const char *fs, GLuint *shaderProgram)
{
    // Compile the shaders and link the program
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
        *shaderProgram = glCreateProgram();
        glAttachShader(*shaderProgram, vertexShader);
        glAttachShader(*shaderProgram, fragmentShader);
        glBindFragDataLocation(*shaderProgram, 0, "outColor");
        glLinkProgram(*shaderProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
}

class Material
{
public:
    Material(const char *vs, const char *fs)
    {
        readAndCompileShaders(vs, fs, &shaderProgram);

        // Get the number of uniforms in shaderProgram
        glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORMS, &numUniforms);
        uniformDesc = (UniformDesc *)malloc(sizeof(UniformDesc) * numUniforms);

        // Populate uniformDesc
        GLsizei lengthWritten;
        for(GLint i = 0; i < numUniforms; i++)
        {
            uniformDesc[i].index = i;
            glGetActiveUniform(shaderProgram, uniformDesc[i].index, 20, &lengthWritten,
                               &(uniformDesc[i].size), &(uniformDesc[i].type), uniformDesc[i].name);
            assert((lengthWritten + 1) < 20);
            uniformDesc[i].handle = glGetUniformLocation(shaderProgram, uniformDesc[i].name);
        }

        // Retrieve handles to vertex attributes
        h_aPosition = glGetAttribLocation(shaderProgram, "aPosition");
        h_aNormal = glGetAttribLocation(shaderProgram, "aNormal");
        h_aTexcoord = glGetAttribLocation(shaderProgram, "aTexcoord");
    }

    ~Material()
    {
        for(GLint i = 0; i < numUniforms; i++)
        {
            free(uniformDesc);
            //free(uniforms[i]);
        }
        glDeleteProgram(shaderProgram);
    }

    GLint getNumUniforms()
    {
        return numUniforms;
    }

    bool sendUniform1i(const char *uniformName, GLint uniform)
    {
        GLint i;
        for(i = 0; i < numUniforms; i++)
        {
            if(strcmp(uniformDesc[i].name, uniformName) == 0)
                break;
        }
        if(i == numUniforms)
        {
            if(g_debugUniformString == true)
                fprintf(stderr, "No active uniform %s.\n", uniformName);
            return false;
        }

        glUseProgram(shaderProgram);
        glUniform1i(uniformDesc[i].handle, uniform);
        glUseProgram(0);
    }

    bool sendUniform3v(const char *uniformName, Vec3 uniform)
    {
        GLint i;
        for(i = 0; i < numUniforms; i++)
        {
            if(strcmp(uniformDesc[i].name, uniformName) == 0)
                break;
        }
        if(i == numUniforms)
        {
            if(g_debugUniformString == true)
                fprintf(stderr, "No active uniform %s.\n", uniformName);
            return false;
        }

        glUseProgram(shaderProgram);
        glUniform3f(uniformDesc[i].handle, uniform[0], uniform[1], uniform[2]);
        glUseProgram(0);
        return true;
    }

    // TODO: change Mat4 to RigTForm to keep a more consistent interface?
    bool sendUniformMat4(const char *uniformName, Mat4 uniform)
    {
        GLint i;
        for(i = 0; i < numUniforms; i++)
        {
            if(strcmp(uniformDesc[i].name, uniformName) == 0)
                break;
        }
        if(i == numUniforms)
        {
            if(g_debugUniformString == true)
                fprintf(stderr, "No active uniform %s.\n", uniformName);
            return false;
        }

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(uniformDesc[i].handle, 1, GL_FALSE, &(uniform[0]));
        glUseProgram(0);
        return true;
    }
    
    bool sendUniformTexture(const char *uniformName, GLuint uniform, GLenum texture, GLint texUnit)
    {
        GLint i;
        for(i = 0; i < numUniforms; i++)
        {
            if(strcmp(uniformDesc[i].name, uniformName) == 0)
                break;
        }
        if(i == numUniforms)
        {
            if(g_debugUniformString == true)
                fprintf(stderr, "No active uniform %s.\n", uniformName);
            return false;
        }
        
        glUseProgram(shaderProgram);
        glActiveTexture(texture);
        glBindTexture(GL_TEXTURE_2D, uniform);
        glUniform1i(uniformDesc[i].handle, texUnit);
        glUseProgram(0);
    }

    void draw(Geometry *geometry, RigTForm& modelViewRbt)
    {
        glBindVertexArray(geometry->vao);
        glBindBuffer(GL_ARRAY_BUFFER, geometry->vbo);

        Mat4 modelViewMat = rigTFormToMat(modelViewRbt);
        modelViewMat = transpose(modelViewMat);
        
        Mat4 normalMat = inv(transpose(modelViewMat));
        
        glUseProgram(shaderProgram);
        /*
        glUniformMatrix4fv(h_uModelViewMat, 1, GL_FALSE, &(modelViewMat[0]));
        glUniformMatrix4fv(h_uNormalMat, 1, GL_FALSE, &(normalMat[0]));
        */

        GLint i;
        for(i = 0; i < numUniforms; i++)
            if(strcmp(uniformDesc[i].name, "uModelViewMat") == 0)
                break;
        glUniformMatrix4fv(uniformDesc[i].handle, 1, GL_FALSE, &(modelViewMat[0]));

        for(i = 0; i < numUniforms; i++)
            if(strcmp(uniformDesc[i].name, "uNormalMat") == 0)
                break;
        glUniformMatrix4fv(uniformDesc[i].handle, 1, GL_FALSE, &(normalMat[0]));


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

        glUseProgram(0);
    }
//private:
    GLuint shaderProgram;
    //Uniform *uniforms;
    UniformDesc *uniformDesc;
    GLint numUniforms;
    GLuint h_aPosition, h_aNormal, h_aTexcoord;
};

/*
struct ShaderState {

    GLuint shaderProgram;

    // Handles to uniforms
    GLint h_uModelViewMat, h_uNormalMat, h_uProjMat, h_uColor, h_uLight;

    // Handles to attributes
    GLint h_aPosition, h_aNormal, h_aTexcoord;
    

    ShaderState(const char* vs, const char* fs) {
        readAndCompileShaders(vs, fs, &shaderProgram);

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


        if (!g_Gl2Compatible)
            glBindFragDataLocation(h, 0, "fragColor");
        checkGlErrors();

    }

    void draw(Geometry *geometry, RigTForm& modelViewRbt)
    {
        glBindVertexArray(geometry->vao);
        glBindBuffer(GL_ARRAY_BUFFER, geometry->vbo);

        Mat4 modelViewMat = rigTFormToMat(modelViewRbt);
        modelViewMat = transpose(modelViewMat);
        
        Mat4 normalMat = inv(transpose(modelViewMat));
        
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(h_uModelViewMat, 1, GL_FALSE, &(modelViewMat[0]));
        glUniformMatrix4fv(h_uNormalMat, 1, GL_FALSE, &(normalMat[0]));

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

        glUseProgram(0);
    }


    // Note: Having so many glUseProgram() might be expensive.
    void sendLightEyePos(Vec3& lightE)
    {
        glUseProgram(shaderProgram);
        glUniform3f(h_uLight, lightE[0], lightE[1], lightE[2]);
        glUseProgram(0);        
    }

    void sendColor(Vec3& color)
    {
        glUseProgram(shaderProgram);
        glUniform3f(h_uColor, color[0], color[1], color[2]);
        glUseProgram(0);        
    }
};
*/


/*
struct ShaderState {

    GLuint shaderProgram;

    // Handles to uniforms
    GLint h_uModelViewMat, h_uNormalMat, h_uProjMat, h_uColor, h_uLight;

    // Handles to attributes
    GLint h_aPosition, h_aNormal, h_aTexcoord;
    

    ShaderState(const char* vs, const char* fs) {
        readAndCompileShaders(vs, fs, &shaderProgram);
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
