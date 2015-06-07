#ifndef MATERIAL_H
#define MATERIAL_H

#include <GL/glew.h>

#include <string.h>
//#include "SOIL.h"
#include "rigtform.h"
#include "geometry.h"

static bool g_debugUniformString = true;

/*
struct AttribDesc
{
    char name[20];
    GLuint index;
    GLenum type;
    GLuint handle;
    GLsizei size;
};
*/

enum VertexAttrib{
    PNX,                      // position, normal, texcoord
    PNXTBD                    // position, normal, texcoord, tangent, binormal, determinant
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
    Material(const char *vs, const char *fs, const char *materialName)
    {
        strcpy(name, materialName);
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

        // Retrieve handles to the basic vertex attributes
        h_aPosition = glGetAttribLocation(shaderProgram, "aPosition");
        h_aNormal = glGetAttribLocation(shaderProgram, "aNormal");
        h_aTexcoord = glGetAttribLocation(shaderProgram, "aTexcoord");

        // Retrieve handles to optional vertex attributes
        GLint numActiveAttrib;
        glGetProgramiv(shaderProgram, GL_ACTIVE_ATTRIBUTES, &numActiveAttrib);
        printf("numActiveAttrib = %d\n", numActiveAttrib);
        if(numActiveAttrib == 6)
        {
            printf("Optional attributes\n");
            h_aTangent = glGetAttribLocation(shaderProgram, "aTangent");
            h_aBinormal = glGetAttribLocation(shaderProgram, "aBinormal");
            h_aDet = glGetAttribLocation(shaderProgram, "aDet");
            vertexAttrib = PNXTBD;
        }else
        {
            vertexAttrib = PNX;            
        }
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

    char* getName()
    {
        return name;
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
            if(g_debugUniformString)
                fprintf(stderr, "No active uniform %s.\n", uniformName);
            return false;
        }

        glUseProgram(shaderProgram);
        glUniform1i(uniformDesc[i].handle, uniform);
        glUseProgram(0);
        return true;
    }

    bool sendUniform1f(const char *uniformName, GLfloat uniform)
    {
        GLint i;
        for(i = 0; i < numUniforms; i++)
        {
            if(strcmp(uniformDesc[i].name, uniformName) == 0)
                break;
        }
        if(i == numUniforms)
        {
            if(g_debugUniformString)
                fprintf(stderr, "No active uniform %s.\n", uniformName);
            return false;
        }

        glUseProgram(shaderProgram);
        glUniform1f(uniformDesc[i].handle, uniform);
        glUseProgram(0);
        return true;
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
            if(g_debugUniformString)
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
            if(g_debugUniformString)
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
            if(g_debugUniformString)
                fprintf(stderr, "No active uniform %s.\n", uniformName);
            return false;
        }
        
        glUseProgram(shaderProgram);
        glActiveTexture(texture);
        glBindTexture(GL_TEXTURE_2D, uniform);
        glUniform1i(uniformDesc[i].handle, texUnit);
        glUseProgram(0);

        return true;
    }

    void draw(Geometry *geometry, RigTForm& modelViewRbt, Vec3& scaleFactor)
    {
        glBindVertexArray(geometry->vao);
        glBindBuffer(GL_ARRAY_BUFFER, geometry->vbo);

        Mat4 scaleMat;
        scaleMat[0] = scaleFactor[0];
        scaleMat[5] = scaleFactor[1];
        scaleMat[10] = scaleFactor[2];

        Mat4 modelViewMat = rigTFormToMat(modelViewRbt); 
        modelViewMat = transpose(modelViewMat);
        
        Mat4 normalMat = inv(transpose(modelViewMat));
        /* NOTE: tacked scaleMat here, because if I do modelViewMat = rigTFormToMat(modelViewRbt) * scaleMat,
           then Mat4 normalMat = inv(transpose(modelViewMat)), the teapot becomes really bright, and texture isn't really
           visible anymore. So I assume something wrong happened to the normals when I do that. scaleMat doesn't need to be
           transposed because it's a diagonal matrix.
         */
        modelViewMat = scaleMat * modelViewMat;
        glUseProgram(shaderProgram);

        for(GLint i = 0; i < numUniforms; i++)
        {
            if(strcmp(uniformDesc[i].name, "uModelViewMat") == 0)
            {
                glUniformMatrix4fv(uniformDesc[i].handle, 1, GL_FALSE, &(modelViewMat[0]));
                break;
            }
        }


        for(GLint i = 0; i < numUniforms; i++)
        {
            if(strcmp(uniformDesc[i].name, "uNormalMat") == 0)
            {
                glUniformMatrix4fv(uniformDesc[i].handle, 1, GL_FALSE, &(normalMat[0]));                
                break;
            }
        }
        

        if(geometry->shaderProgram != shaderProgram)
        {
            GLint vertexSize; 
            if(vertexAttrib == PNX)
                vertexSize = 8;
            else if(vertexAttrib == PNXTBD)
                vertexSize = 15;
            
            glEnableVertexAttribArray(h_aPosition);
            glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, vertexSize * sizeof(GLfloat), 0);
            glEnableVertexAttribArray(h_aNormal);
            glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, vertexSize * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
            glEnableVertexAttribArray(h_aTexcoord);
            glVertexAttribPointer(h_aTexcoord, 2, GL_FLOAT, GL_FALSE, vertexSize * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

            if(vertexAttrib == PNXTBD)
            {
                printf("vertexAttrib == PNXTBD\n");
                glEnableVertexAttribArray(h_aTangent);
                glVertexAttribPointer(h_aTangent, 3, GL_FLOAT, GL_FALSE, vertexSize * sizeof(GLfloat), (void*)(8 * sizeof(GLfloat)));
                glEnableVertexAttribArray(h_aBinormal);
                glVertexAttribPointer(h_aBinormal, 3, GL_FLOAT, GL_FALSE, vertexSize * sizeof(GLfloat), (void*)(11 * sizeof(GLfloat)));
                glEnableVertexAttribArray(h_aDet);
                glVertexAttribPointer(h_aDet, 1, GL_FLOAT, GL_FALSE, vertexSize * sizeof(GLfloat), (void*)(14 * sizeof(GLfloat)));
            }else
                printf("vertexAttrib == PNX\n");
            
            geometry->shaderProgram = shaderProgram;
        }

        if(geometry->eboLen == 0)
            glDrawArrays(GL_TRIANGLES, 0, geometry->vboLen);
        else
            glDrawElements(GL_TRIANGLES, geometry->eboLen, GL_UNSIGNED_INT, 0);

        glUseProgram(0);
    }
private:
    GLuint shaderProgram;
    VertexAttrib vertexAttrib;
    UniformDesc *uniformDesc;
    GLint numUniforms;
    GLuint h_aPosition, h_aNormal, h_aTexcoord;
    GLuint h_aTangent, h_aBinormal, h_aDet;
    char name[20];
};

#endif       
