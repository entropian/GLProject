#ifndef MATERIAL_H
#define MATERIAL_H

#include <GL/glew.h>

#include <string.h>
#include "rigtform.h"
#include "geometry.h"

static bool g_debugUniformString = true;
static const unsigned MAX_TEXTURES_PER_MATERIAL = 32;                // Arbitrary
static const unsigned MAX_NUM_UNIFORMS = 30;


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

static void compileShader(const char* shaderSrc, GLuint &shaderHandle, GLenum shaderType)
{
    shaderHandle = glCreateShader(shaderType);
    glShaderSource(shaderHandle, 1, &shaderSrc, NULL);
    glCompileShader(shaderHandle);

    GLint status;
    GLchar infoLog[512];
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &status);

    if(status != GL_TRUE)
    {
        char str[10];
        switch(shaderType)
        {
        case GL_VERTEX_SHADER:
            strcpy(str, "Vertex");
            break;
        case GL_FRAGMENT_SHADER:
            strcpy(str, "Fragment");            
            break;
        case GL_GEOMETRY_SHADER:
            strcpy(str, "Geometry");            
            break;
        }
        glGetShaderInfoLog(shaderHandle, 512, NULL, infoLog);
        fprintf(stderr, "%s shader compiled incorrectly.\n", str);
        fprintf(stderr, "%s\n", infoLog);
    }    
}

static GLuint compileAndLinkShaders(const char *vs, const char *fs)
{
    GLuint vertexShader, fragmentShader;
    compileShader(vs, vertexShader, GL_VERTEX_SHADER);
    compileShader(fs, fragmentShader, GL_FRAGMENT_SHADER);        

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
        //printf("%s\n%s", vs, fs);
        fprintf(stderr, "%s\n", infoLog);
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

static GLuint compileAndLinkShaders(const char *vs, const char *fs, const char *gs)
{
    GLuint vertexShader, fragmentShader, geometryShader;
    compileShader(vs, vertexShader, GL_VERTEX_SHADER);
    compileShader(fs, fragmentShader, GL_FRAGMENT_SHADER);
    compileShader(gs, geometryShader, GL_GEOMETRY_SHADER);    

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glAttachShader(shaderProgram, geometryShader);
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
    glDeleteShader(geometryShader);    

    return shaderProgram;
}


class Material
{
public:
    Material(const char *vertexShader, const char *fragmentShader, const char *materialName);

    Material(const char *vertexShader, const char *fragmentShader, const char *geometryShader,
             const char *materialName);

    ~Material();

    Material(const Material &mat);

    Material& operator= (const Material &rhs);
    
    char* getName();
    
    GLint getNumUniforms();

    bool sendUniform1i(const char *uniformName, GLint uniform);

    bool sendUniform1f(const char *uniformName, GLfloat uniform);
    
    bool sendUniform3f(const char *uniformName, Vec3 uniform);

    // TODO: change Mat4 to RigTForm to keep a more consistent interface?
    bool sendUniformMat4(const char *uniformName, Mat4 uniform);
    /*
      Instead of directly sending the uniform to the shader program like the other
      sendUniform functions, this function only stores the texture handles.
      Binding of textures and texture units happen in draw()
     */
    bool sendUniformTexture(const char *uniformName, GLuint uniform);

    bool sendUniformCubemap(const char *uniformName, GLuint uniform);

    /*
      Draws geometry with modelview transform modelVewRbt and scaled with scaleFactor
     */
    void draw(Geometry *geometry, const RigTForm &modelRbt, const RigTForm &viewRbt, Vec3& scaleFactor);

    void bindUniformBlock(const char* blockName, GLuint bindingPoint);
    // TODO: temporary
    void setDepthMap(bool b);

private:
    int searchUniformDesc(const char* uniformName);
    void assign(Material &dest, const Material &src);
    void sendMatrix(const char* uniformName, Mat4 &matrix);    
    void initialize();

    
    
    GLuint shaderProgram;
    VertexAttrib vertexAttrib;
    UniformDesc uniformDesc[MAX_NUM_UNIFORMS];
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
