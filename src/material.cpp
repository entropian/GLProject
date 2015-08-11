#include "material.h"
Material::Material(const char *vertexShader, const char *fragmentShader, const char *materialName)
 {
    strcpy(name, materialName);
    shaderProgram = compileAndLinkShaders(vertexShader, fragmentShader);             // Compile shaders    
    initialize();
}

Material::Material(const char *vertexShader, const char *fragmentShader, const char *geometryShader, const char *materialName)
{
    strcpy(name, materialName);
    shaderProgram = compileAndLinkShaders(vertexShader, fragmentShader, geometryShader);    // Compile shaders
    initialize();
}

Material::~Material()
{
    //delete uniformDesc;
    glDeleteProgram(shaderProgram);
}

void Material::assign(Material &dest, const Material &src)
{    
    dest.shaderProgram = src.shaderProgram;
    dest.vertexAttrib = src.vertexAttrib;
    dest.numUniforms = src.numUniforms;
    
    for(int i = 0; i < numUniforms; i++)
    {
        strcpy(dest.uniformDesc[i].name, src.uniformDesc[i].name);
        dest.uniformDesc[i].index = src.uniformDesc[i].index;
        dest.uniformDesc[i].type = src.uniformDesc[i].type;
        dest.uniformDesc[i].handle = src.uniformDesc[i].handle;
        dest.uniformDesc[i].size = src.uniformDesc[i].size;       
    }
    dest.h_aPosition = src.h_aPosition;
    dest.h_aNormal = src.h_aNormal;
    dest.h_aTexcoord = src.h_aTexcoord;

    if(dest.vertexAttrib == PNXTBD)
    {
        dest.h_aTangent = src.h_aTangent;
        dest.h_aBinormal = src.h_aBinormal;
        dest.h_aDet = src.h_aDet;  
    }
    dest.textureCount = src.textureCount;
    dest.cubemap = src.cubemap;
    dest.depthMap = src.depthMap;    
    
    if(dest.cubemap)
        dest.cubemapHandle = src.cubemapHandle;
    strcpy(dest.name, src.name);
}

Material::Material(const Material &mat)
{
    assign(*this, mat);
}

Material& Material::operator= (const Material &rhs)
{
    assign(*this, rhs);
    return *this;
}

char* Material::getName()
{
    return name;
}
    
GLint Material::getNumUniforms()
{
    return numUniforms;
}

bool Material::sendUniform1i(const char *uniformName, GLint uniform)
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

bool Material::sendUniform1f(const char *uniformName, GLfloat uniform)
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
    
bool Material::sendUniform3f(const char *uniformName, Vec3 uniform)
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
bool Material::sendUniformMat4(const char *uniformName, Mat4 uniform)
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
bool Material::sendUniformTexture(const char *uniformName, GLuint uniform)
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

bool Material::sendUniformCubemap(const char *uniformName, GLuint uniform)
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
void Material::draw(Geometry *geometry, const RigTForm &modelRbt, const RigTForm &viewRbt, Vec3& scaleFactor)
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
        /*
        // TODO: only for shadow map testing.
        Mat4 modelMat = rigTFormToMat(modelRbt);
        Mat4 viewMat = rigTFormToMat(viewRbt);            
        Mat4 normalMat = transpose(inv(modelMat));
        modelMat = modelMat * scaleMat;

        sendMatrix("uModelMat", modelMat);
        sendMatrix("uViewMat", viewMat);
        sendMatrix("uNormalMat", normalMat);
        */

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
    
void Material::bindUniformBlock(const char* blockName, GLuint bindingPoint)
{
    GLuint blockIndex = glGetUniformLocation(shaderProgram, blockName);
    glUniformBlockBinding(shaderProgram, blockIndex, bindingPoint);
}


int Material::searchUniformDesc(const char* uniformName)
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
            fprintf(stderr, "No active uniform %s in %s.\n", uniformName, name);

        return -1;
    }
    return i;
}

void Material::setDepthMap(bool b)
{
    depthMap = b;
}

void Material::sendMatrix(const char* uniformName, Mat4 &matrix)
{
    for(int i = 0; i < numUniforms; i++)
        if(strcmp(uniformDesc[i].name, uniformName) == 0)
        {
            glUniformMatrix4fv(uniformDesc[i].handle, 1, GL_TRUE, &(matrix[0]));
            break;
        }
}

void Material::initialize()
{
    glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORMS, &numUniforms);          // Get the number of uniforms in shaderProgram
    if(numUniforms > MAX_NUM_UNIFORMS)
        fprintf(stderr, "Too many active uniforms in material %s\n", name);
    //uniformDesc = (UniformDesc *)malloc(sizeof(UniformDesc) * numUniforms);

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
    cubemap = false;
    depthMap = false;
    textureCount = 0;
}
