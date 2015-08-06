#ifndef MATERIAL_H
#define MATERIAL_H

#include <GL/glew.h>

#include <string.h>
#include "rigtform.h"
#include "geometry.h"

static bool g_debugUniformString = true;
static const unsigned MAX_TEXTURES_PER_MATERIAL = 32;                // Arbitrary

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

static void compileVertFragShaders(const char *vs, const char *fs, GLuint &vertexShader, GLuint &fragmentShader)
{
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vs, NULL);
    glCompileShader(vertexShader);

    GLint status;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);

    if(status != GL_TRUE)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        fprintf(stderr, "Vertex shader compiled incorrectly.\n");
        fprintf(stderr, "%s\n", infoLog);
    }

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fs, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);

    if(status != GL_TRUE)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        fprintf(stderr, "Fragment shader compiled incorrectly.\n");
        fprintf(stderr, "%s\n", infoLog);
    }    
}

static GLuint compileAndLinkShaders(const char *vs, const char *fs)
{
    GLuint vertexShader, fragmentShader;
    compileVertFragShaders(vs, fs, vertexShader, fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);

    GLint status;
    GLchar infoLog[512];    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if(status != GL_TRUE)
    {
        glGetProgramInfoLog(fragmentShader, 512, NULL, infoLog);
        fprintf(stderr, "Shaders linked incorrectly.\n");
        fprintf(stderr, "%s\n", infoLog);
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

static GLuint compileAndLinkShaders(const char *vs, const char *fs, const char *gs)
{
    GLuint vertexShader, fragmentShader;
    compileVertFragShaders(vs, fs, vertexShader, fragmentShader);

    GLuint geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometryShader, 1, &gs, NULL);
    glCompileShader(geometryShader);

    GLint status;
    GLchar infoLog[512];
    glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &status);

    if(status != GL_TRUE)
    {
        glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
        fprintf(stderr, "Geometry shader compiled incorrectly.\n");
        fprintf(stderr, "%s\n", infoLog);
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glAttachShader(shaderProgram, geometryShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if(status != GL_TRUE)
    {
        glGetProgramInfoLog(fragmentShader, 512, NULL, infoLog);
        fprintf(stderr, "Shaders linked incorrectly.\n");
        fprintf(stderr, "%s\n", infoLog);
    }    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(geometryShader);    

    return shaderProgram;
}

/*
// Compile vertex shader vs and fragment shader fs, and attach them to a shader program
// Returns the handle to the shader program
static GLuint compileAndLinkShaders(const char *vs, const char *fs)
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vs, NULL);
    glCompileShader(vertexShader);

    GLint status;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);

    if(status != GL_TRUE)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        fprintf(stderr, "Vertex shader compiled incorrectly.\n");
        fprintf(stderr, "%s\n", infoLog);
    }
        
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fs, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);

    if(status != GL_TRUE)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        fprintf(stderr, "Fragment shader compiled incorrectly.\n");
        fprintf(stderr, "%s\n", infoLog);
    }


    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if(status != GL_TRUE)
    {
        glGetProgramInfoLog(fragmentShader, 512, NULL, infoLog);
        fprintf(stderr, "Shaders linked incorrectly.\n");
        fprintf(stderr, "%s\n", infoLog);
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}
*/



class Material
{
public:
    Material(const char *vertexShader, const char *fragmentShader, const char *materialName)
    {
        strcpy(name, materialName);
        shaderProgram = compileAndLinkShaders(vertexShader, fragmentShader);             // Compile shaders

        glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORMS, &numUniforms);          // Get the number of uniforms in shaderProgram
        uniformDesc = (UniformDesc *)malloc(sizeof(UniformDesc) * numUniforms);

        // Populate uniformDesc with uniform information
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
        vertexAttrib = PNX;            

        // Retrieve handles to optional vertex attributes
        GLint numActiveAttrib;
        glGetProgramiv(shaderProgram, GL_ACTIVE_ATTRIBUTES, &numActiveAttrib);
        if(numActiveAttrib == 6)
        {
            h_aTangent = glGetAttribLocation(shaderProgram, "aTangent");
            h_aBinormal = glGetAttribLocation(shaderProgram, "aBinormal");
            h_aDet = glGetAttribLocation(shaderProgram, "aDet");
            vertexAttrib = PNXTBD;
        }
        textureCount = 0;
        cubemap = false;
        depthMap = false;
    }

    Material(const char *vertexShader, const char *fragmentShader, const char *geometryShader, const char *materialName)
    {
        strcpy(name, materialName);
        shaderProgram = compileAndLinkShaders(vertexShader, fragmentShader, geometryShader);    // Compile shaders

        glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORMS, &numUniforms);          // Get the number of uniforms in shaderProgram
        uniformDesc = (UniformDesc *)malloc(sizeof(UniformDesc) * numUniforms);

        // Populate uniformDesc with uniform information
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
        vertexAttrib = PNX;            

        // Retrieve handles to optional vertex attributes
        GLint numActiveAttrib;
        glGetProgramiv(shaderProgram, GL_ACTIVE_ATTRIBUTES, &numActiveAttrib);
        if(numActiveAttrib == 6)
        {
            h_aTangent = glGetAttribLocation(shaderProgram, "aTangent");
            h_aBinormal = glGetAttribLocation(shaderProgram, "aBinormal");
            h_aDet = glGetAttribLocation(shaderProgram, "aDet");
            vertexAttrib = PNXTBD;
        }
        textureCount = 0;
        cubemap = false;
        depthMap = false;
    }

    ~Material()
    {
        for(int i = 0; i < numUniforms; i++)
        {
            free(uniformDesc);
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
        int i = searchUniformDesc(uniformName);
        if(i != -1)
        {
            glUseProgram(shaderProgram);
            glUniform1i(uniformDesc[i].handle, uniform);
            glUseProgram(0);
            return true;
        }
        return false;
    }

    bool sendUniform1f(const char *uniformName, GLfloat uniform)
    {
        int i = searchUniformDesc(uniformName);
        if(i != -1)
        {
            glUseProgram(shaderProgram);
            glUniform1f(uniformDesc[i].handle, uniform);
            glUseProgram(0);
            return true;
        }
        return false;
    }
    
    bool sendUniform3f(const char *uniformName, Vec3 uniform)
    {
        int i = searchUniformDesc(uniformName);
        if(i != -1)
        {
            glUseProgram(shaderProgram);
            glUniform3f(uniformDesc[i].handle, uniform[0], uniform[1], uniform[2]);
            glUseProgram(0);
            return true;
        }
        return false;
    }

    // TODO: change Mat4 to RigTForm to keep a more consistent interface?
    bool sendUniformMat4(const char *uniformName, Mat4 uniform)
    {
        int i = searchUniformDesc(uniformName);
        if(i != -1)
        {
            glUseProgram(shaderProgram);
            glUniformMatrix4fv(uniformDesc[i].handle, 1, GL_TRUE, &(uniform[0]));
            glUseProgram(0);
            return true;
        }
        return false;
    }

    /*
      Instead of directly sending the uniform to the shader program like the other
      sendUniform functions, this function only stores the texture handles.
      Binding of textures and texture units happen in draw()
     */
    bool sendUniformTexture(const char *uniformName, GLuint uniform)
    {
        int i = searchUniformDesc(uniformName);
        if(i != -1)
        {
            textureHandles[textureCount] = uniform;
            textureToUniformIndex[textureCount] = i;
            ++textureCount;
            return true;
        }
        return false;
    }

    bool sendUniformCubemap(const char *uniformName, GLuint uniform)
    {
        int i = searchUniformDesc(uniformName);

        if(i != -1)
        {
            cubemapHandle = uniform;
            cubemap = true;
            return true;
        }
        return false;
    }

    /*
      Draws geometry with modelview transform modelVewRbt and scaled with scaleFactor
     */
    void draw(Geometry *geometry, const RigTForm &modelRbt, const RigTForm &viewRbt, Vec3& scaleFactor)
    {
        /*
          // Preliminary messing about with culling while having no idea how to do culling.
          // Does not work well.
          Mat4 viewMat = rigTFormToMat(viewRbt);
          // NOTE: Look more into matrix inversion
          Mat4 invViewMat = inv(viewMat);
          Vec3 z(invViewMat(0, 2), invViewMat(1, 2), invViewMat(2, 2));
          Vec3 modelDir = modelRbt.getTranslation() - Vec3(invViewMat(0, 3), invViewMat(1, 3), invViewMat(2, 3));
          if(dot(modelDir, z) < 0)
        */

        glBindVertexArray(geometry->vao);
        // NOTE: don't know why the vbo needs rebinding
        glBindBuffer(GL_ARRAY_BUFFER, geometry->vbo);

        Mat4 scaleMat;
        scaleMat[0] = scaleFactor[0];
        scaleMat[5] = scaleFactor[1];
        scaleMat[10] = scaleFactor[2];

        glUseProgram(shaderProgram);        

        if(cubemap)
        {
            Mat4 modelMat = rigTFormToMat(modelRbt);
            Mat4 viewMat = rigTFormToMat(viewRbt);            
            Mat4 normalMat = transpose(inv(modelMat));
            modelMat = modelMat * scaleMat;

            sendMatrix("uModelMat", modelMat);
            sendMatrix("uViewMat", viewMat);
            sendMatrix("uNormalMat", normalMat);               


        }else if(depthMap)
        {
            Mat4 modelMat = rigTFormToMat(modelRbt);
            sendMatrix("uModelMat", modelMat);
        }else
        {
            RigTForm modelViewRbt = viewRbt * modelRbt;                
            Mat4 modelViewMat = rigTFormToMat(modelViewRbt); 
            Mat4 normalMat = transpose(inv(modelViewMat));
            modelViewMat = modelViewMat * scaleMat;        

            sendMatrix("uModelViewMat", modelViewMat);
            sendMatrix("uNormalMat", normalMat);
        }

        /*
          Binding texture objects to texture units and binding texture units
          to uniforms
        */
        for(int i = 0; i < textureCount; i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textureHandles[i]);
            glUniform1i(uniformDesc[textureToUniformIndex[i]].handle, i);
        }
        
        if(cubemap)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapHandle);
            for(int i = 0; i < numUniforms; i++)
                if(strcmp(uniformDesc[i].name, "uCubemap") == 0)
                {
                    glUniform1i(uniformDesc[i].handle, 0);
                    break;
                }
        }
        
        // Link vertex attributes 
        if(geometry->shaderProgram != shaderProgram)
        {
            /*
              GLint vertexSize;
              if(vertexAttrib == PNX)
              vertexSize = 8;
              else if(vertexAttrib == PNXTBD)
              vertexSize = 15;
            */
            
            glEnableVertexAttribArray(h_aPosition);
            glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, geometry->vertexSize * sizeof(GLfloat), 0);
            glEnableVertexAttribArray(h_aNormal);
            glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, geometry->vertexSize * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
            glEnableVertexAttribArray(h_aTexcoord);
            glVertexAttribPointer(h_aTexcoord, 2, GL_FLOAT, GL_FALSE, geometry->vertexSize * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

            if(vertexAttrib == PNXTBD)
            {
                glEnableVertexAttribArray(h_aTangent);
                glVertexAttribPointer(h_aTangent, 3, GL_FLOAT, GL_FALSE, geometry->vertexSize * sizeof(GLfloat), (void*)(8 * sizeof(GLfloat)));
                glEnableVertexAttribArray(h_aBinormal);
                glVertexAttribPointer(h_aBinormal, 3, GL_FLOAT, GL_FALSE, geometry->vertexSize * sizeof(GLfloat), (void*)(11 * sizeof(GLfloat)));
                glEnableVertexAttribArray(h_aDet);
                glVertexAttribPointer(h_aDet, 1, GL_FLOAT, GL_FALSE, geometry->vertexSize * sizeof(GLfloat), (void*)(14 * sizeof(GLfloat)));
            }
            
            geometry->shaderProgram = shaderProgram;
        }

        if(geometry->eboLen == 0)
            glDrawArrays(GL_TRIANGLES, 0, geometry->vboLen);
        else
            glDrawElements(GL_TRIANGLES, geometry->eboLen, GL_UNSIGNED_INT, 0);

        glUseProgram(0);
    }
    
    void bindUniformBlock(const char* blockName, GLuint bindingPoint)
    {
        GLuint blockIndex = glGetUniformLocation(shaderProgram, blockName);
        glUniformBlockBinding(shaderProgram, blockIndex, bindingPoint);
    }

    // TODO: temporary
    void setDepthMap(bool b)
    {
        depthMap = b;
    }
private:
    int searchUniformDesc(const char* uniformName)
    {
        int i;
        for(i = 0; i < numUniforms; i++)
        {
            if(strcmp(uniformDesc[i].name, uniformName) == 0)
                break;
        }
        if(i == numUniforms)
        {            
            if(g_debugUniformString)
                fprintf(stderr, "No active uniform %s.\n", uniformName);
            return -1;
        }
        return i;
    }

    void sendMatrix(const char* uniformName, Mat4 &matrix)
    {
        for(int i = 0; i < numUniforms; i++)
            if(strcmp(uniformDesc[i].name, uniformName) == 0)
            {
                glUniformMatrix4fv(uniformDesc[i].handle, 1, GL_TRUE, &(matrix[0]));
                break;
            }
    }
    
    
    GLuint shaderProgram;
    VertexAttrib vertexAttrib;
    UniformDesc *uniformDesc;
    int numUniforms;
    GLuint h_aPosition, h_aNormal, h_aTexcoord;
    GLuint h_aTangent, h_aBinormal, h_aDet;
    GLuint textureToUniformIndex[MAX_TEXTURES_PER_MATERIAL];
    GLuint textureHandles[MAX_TEXTURES_PER_MATERIAL];
    unsigned int textureCount;
    GLuint cubemapHandle;
    bool cubemap, depthMap;    
    char name[20];
};

#endif       
