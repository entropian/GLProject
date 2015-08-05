#ifndef SHADERS_H
#define SHADERS_H

#include <GL/glew.h>


#define GLSL(src) "#version 330 core\n" #src
//-------------------------------------------- Shaders
//------------------- Vertex Shaders

const char* RTBVertSrc = GLSL(
    layout (location = 0) in vec2 aPosition;
    layout (location = 1) in vec2 aTexcoord;

    out vec2 vTexcoord;
    
    void main()
    {
        gl_Position = vec4(aPosition, 0.0f, 1.0f);
        vTexcoord = aTexcoord;
    }
);

const char* skyboxVertSrc = GLSL(
    layout (location = 0) in vec3 aPosition;
    out vec3 Texcoords;

    layout (std140) uniform UniformBlock
    {
                            // Base alignment    Aligned offset
        mat4 projMat;       // 16 (x4)           0
        vec3 light1;        // 16                64
    };            
    
    uniform mat4 uViewMat;

    void main()
    {
        gl_Position = projMat * uViewMat * vec4(aPosition, 1.0);
        Texcoords = aPosition;
    }
);

const char* testVertSrc = GLSL(
    in vec3 aPosition;
    in vec3 aNormal;
    in vec2 aTexcoord;

    layout (std140) uniform UniformBlock
    {
                            // Base alignment    Aligned offset
        mat4 projMat;       // 16 (x4)           0
        vec3 light1;        // 16                64
    };
    uniform mat4 uModelViewMat;
    uniform mat4 uNormalMat;

    out VS_OUT {
        vec3 normal;
    } vs_out;
    
    void main()
    {
        gl_Position = projMat * uModelViewMat * vec4(aPosition, 1.0);
        vs_out.normal = normalize(vec3(projMat * uNormalMat * vec4(aNormal, 1.0)));
    }
);



const char* basicVertSrc = GLSL(
    
    uniform mat4 uModelViewMat;
    uniform mat4 uNormalMat;

    layout (std140) uniform UniformBlock
    {
                            // Base alignment    Aligned offset
        mat4 projMat;       // 16 (x4)           0
        vec3 light1;        // 16                64
    };
    
    in vec3 aPosition;
    in vec3 aNormal;
    in vec2 aTexcoord;

    out vec3 vPosition;
    out vec3 vNormal;
    out vec2 vTexcoord;

    void main()
    {        
        vPosition = (uModelViewMat * vec4(aPosition, 1.0)).xyz;
        vNormal = (uNormalMat * vec4(aNormal, 1.0)).xyz;
        vTexcoord = aTexcoord;
        gl_Position = projMat * vec4(vPosition, 1.0);
    }
);

const char* arrowVertSrc = GLSL(
    uniform mat4 uProjMat;
    uniform mat4 uModelViewMat;
    uniform mat4 uNormalMat;

    layout (std140) uniform UniformBlock
    {
                            // Base alignment    Aligned offset
        mat4 projMat;       // 16 (x4)           0
        vec3 light1;        // 16                64
    };                

    in vec3 aPosition;
    in vec3 aNormal;
    in vec2 aTexcoord;

    out vec3 vPosition;
    out vec3 vNormal;
    out vec2 vTexcoord;

    void main()
    {        
        vPosition = (uModelViewMat * vec4(aPosition, 1.0)).xyz;
        vNormal = (uNormalMat * vec4(aNormal, 1.0)).xyz;
        vTexcoord = aTexcoord;
        gl_Position = uProjMat * vec4(vPosition, 1.0);
    }
);

const char* cubemapReflectionVertSrc = GLSL(
    uniform mat4 uModelMat;
    uniform mat4 uViewMat;    
    uniform mat4 uNormalMat;

    layout (std140) uniform UniformBlock
    {
                            // Base alignment    Aligned offset
        mat4 projMat;       // 16 (x4)           0
        vec3 light1;        // 16                64
    };

    in vec3 aPosition;
    in vec3 aNormal;
    in vec2 aTexcoord;

    //out vec3 TexVec;
    out vec3 vPosition;
    out vec3 vNormal;
    out vec3 eyePosW;

    void main()
    {
        vPosition = (uModelMat * vec4(aPosition, 1.0)).xyz;
        vNormal = (uNormalMat * vec4(aNormal, 1.0)).xyz;
        mat4 tmp = inverse(uViewMat);
        /*
          NOTE: changing to tranpose flag in glUniformMatrix4fv from GL_FALSE to GL_TRUE seemed to have changed
          the matrix layout in shaders?
        */
        eyePosW = tmp[3].xyz;
        gl_Position = projMat * uViewMat * vec4(vPosition, 1.0);
    }
);



// Vertex shader that calculates the orthonormal basis for normal mapping
const char* normalVertSrc = GLSL(    
    uniform mat4 uModelViewMat;
    uniform mat4 uNormalMat;

    layout (std140) uniform UniformBlock
    {
                            // Base alignment    Aligned offset
        mat4 projMat;       // 16 (x4)           0
        vec3 light1;        // 16                64
    };                

    in vec3 aPosition;
    in vec3 aNormal;
    in vec2 aTexcoord;

    in vec3 aTangent;
    in vec3 aBinormal;
    in float aDet;

    out vec3 vLightT;
    out vec3 vEyeT;
    out vec3 vLightTarget;
    out vec2 vTexcoord;
    
    void main()
    {
        vTexcoord = aTexcoord;
        mat4 inverseMat = inverse(uModelViewMat);
        vec3 lightM = (inverseMat * vec4(light1, 1.0)).xyz - aPosition;        
        vec3 eyeM = (inverseMat * vec4(0.0, 0.0, 0.0, 1.0)).xyz - aPosition;

        lightM = normalize(lightM);
        eyeM = normalize(eyeM);

        vEyeT.x = dot(eyeM, aTangent);
        vEyeT.y = dot(eyeM, aBinormal);
        vEyeT.z = dot(eyeM, aNormal);
        vEyeT = vEyeT * aDet;
        
        vLightT.x = dot(lightM, aTangent);
        vLightT.y = dot(lightM, aBinormal);
        vLightT.z = dot(lightM, aNormal);
        vLightT = vLightT * aDet;

        gl_Position = projMat * uModelViewMat * vec4(aPosition, 1.0);
    }
);

const char* diffuseVertSrc = GLSL(
    uniform mat4 uModelViewMat;
    uniform mat4 uNormalMat;
    uniform vec3 uColor;

    layout (std140) uniform UniformBlock
    {
                            // Base alignment    Aligned offset
        mat4 projMat;       // 16 (x4)           0
        vec3 light1;        // 16                64
    };                

    in vec3 aPosition;
    in vec3 aNormal;
    in vec2 aTexcoord;

    out vec3 vColor;
    out vec2 vTexcoord;
        
    void main() {
        // in world space
        /*
        vec3 posW = (trans * vec4(position, 1.0)).xyz;
        vec3 normW = (transpose(inverse(trans)) * vec4(normal, 1.0)).xyz;
        vec3 lightDir = normalize(light - posW);
        */

        vec3 posE = (uModelViewMat * vec4(aPosition, 1.0)).xyz;
        //vec3 normE = (inverse(transpose(uModelViewMat)) * vec4(aNormal, 1.0)).xyz;
        vec3 normE = (normalMat * vec4(aNormal, 1.0)).xyz;
        vec3 lightDirE = normalize(light1 - posE);

        vColor = uColor * max(dot(lightDirE, normE), 0.0);

        vTexcoord = aTexcoord;
        gl_Position = projMat * uModelViewMat * vec4(aPosition, 1.0);
    }
);

// Vertex shader with specular lighting
const char* lightVertexSrc = GLSL(
    uniform mat4 uModelViewMat;
    uniform mat4 uNormalMat;
    uniform vec3 uColor;

    layout (std140) uniform UniformBlock
    {
                            // Base alignment    Aligned offset
        mat4 projMat;       // 16 (x4)           0
        vec3 light1;        // 16                64
    };                

    in vec3 aPosition;
    in vec3 aNormal;
    in vec2 aTexcoord;

    out vec3 vColor;
    out vec2 vTexcoord;
        
    void main() {
        // in eye space
        vec3 posE = (uModelViewMat * vec4(aPosition, 1.0)).xyz;
        vec3 normE = (uNormalMat * vec4(aNormal, 1.0)).xyz;
        vec3 lightDirE = normalize(light1 - posE);
        vec3 eyeDirE = normalize(-posE);
        vec3 reflectDirE = 2.0*dot(lightDirE, normE)*normE - lightDirE;

        vColor = uColor * pow(max(dot(eyeDirE, reflectDirE), 0.0), 10);
        
        vTexcoord = aTexcoord;
        gl_Position = projMat * uModelViewMat * vec4(aPosition, 1.0);
    }
);

// Vertex shader for object picking
const char *pickVertSrc = GLSL(
    uniform mat4 uModelViewMat;
    uniform mat4 uProjMat;

    in vec3 aPosition;
    in vec3 aNormal;
    in vec2 aTexcoord;

    void main()
    {
        gl_Position = uProjMat * uModelViewMat * vec4(aPosition, 1.0);
    }
);


//--------------------- Fragment Shaders

const char* RTBFragSrc = GLSL(
    uniform sampler2D uTex0;
    
    in vec2 vTexcoord;

    out vec4 outColor;

    void main()
    {
        outColor = texture(uTex0, vTexcoord);
    }
);

const char* skyboxFragSrc = GLSL(
    in vec3 Texcoords;

    uniform samplerCube skybox;

    out vec4 outColor;

    void main()
    {
        outColor = texture(skybox, Texcoords);
    } 
);

const char* cubemapReflectionFragSrc = GLSL(
    uniform samplerCube uCubemap;

    //in vec3 TexVec;
    in vec3 vPosition;
    in vec3 vNormal;
    in vec3 eyePosW;

    out vec4 outColor;

    void main()
    {

        vec3 eyeDir = normalize(eyePosW - vPosition);
        vec3 reflectDir = 2*dot(eyeDir, vNormal)*vNormal - eyeDir;
        outColor = texture(uCubemap, reflectDir);
    }
);

const char* basicFragSrc2 = GLSL(

    out vec4 outColor;

    void main()
    {
        outColor = vec4(1, 1, 1, 1);
    }
);

const char* fragmentSource = GLSL(
    uniform sampler2D texKitten;
    uniform sampler2D texPuppy;

    in vec3 vColor;
    in vec2 vTexcoord;

    out vec4 outColor;
        
    void main() {
        //outColor = mix(texture(texKitten, Texcoord), texture(texPuppy, Texcoord), 0.5) * vec4(Color, 1.0);
        outColor = vec4(vColor, 1.0);
    }
);

// Fragment shader for object picking
const char *pickFragSrc = GLSL(
    uniform int uCode;

    out vec4 outColor;
    
    void main()
    {
        float color = uCode / 255.0;
        //float color = 1.0;
        outColor = vec4(color, color, color, 1.0);
    }
);

// Fragment shader with diffuse lighting
const char* diffuseFragSrc = GLSL(
    uniform vec3 uColor;

    layout (std140) uniform UniformBlock
    {
                            // Base alignment    Aligned offset
        mat4 projMat;       // 16 (x4)           0
        vec3 light1;        // 16                64
    };                


    in vec3 vPosition;
    in vec3 vNormal;
    in vec3 vTexcoord;

    out vec4 outColor;

    void main()
    {
        vec3 lightDir = normalize(light1 - vPosition);
        vec3 normal = normalize(vNormal);
        outColor = vec4((dot(lightDir, normal) * uColor), 1.0);
    }
);

const char* diffuseGeoFragSrc = GLSL(
    uniform vec3 uColor;

    layout (std140) uniform UniformBlock
    {
                            // Base alignment    Aligned offset
        mat4 projMat;       // 16 (x4)           0
        vec3 light1;        // 16                64
    };                


    in vec3 fPosition;
    in vec3 fNormal;
    in vec3 fTexcoord;

    out vec4 outColor;

    void main()
    {
        vec3 lightDir = normalize(light1 - fPosition);
        vec3 normal = normalize(fNormal);
        outColor = vec4((dot(lightDir, normal) * uColor), 1.0);
    }
);

const char* flatFragSrc = GLSL(
    uniform vec3 uColor;
      
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;

    out vec4 outColor;

    void main()
    {
        outColor = vec4(uColor, 1.0);
    }
);

// Fragment shader with diffuse lighting and texture
const char* basicFragSrc = GLSL(
    uniform vec3 uColor;
    uniform sampler2D uTex0;

    layout (std140) uniform UniformBlock
    {
                            // Base alignment    Aligned offset
        mat4 projMat;       // 16 (x4)           0
        vec3 light1;        // 16                64
    };                
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;

    out vec4 outColor;

    void main()
    {

        vec3 lightDir = normalize(light1 - vPosition);
        vec3 normal = normalize(vNormal);
        vec4 texColor = texture(uTex0, vTexcoord) * vec4(uColor, 1.0);
        outColor = vec4((dot(lightDir, normal) * texColor.xyz), 1.0);
    }
);

// Specular fragment shader with texture
const char* specTexFragSrc = GLSL(
    uniform vec3 uColor;
    uniform sampler2D uTex0;

    layout (std140) uniform UniformBlock
    {
                            // Base alignment    Aligned offset
        mat4 projMat;       // 16 (x4)           0
        vec3 light1;        // 16                64
    };                
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;

    out vec4 outColor;

    void main()
    {

        vec3 lightDir = normalize(light1 - vPosition);
        vec3 reflectDir = 2*dot(lightDir, vNormal)*vNormal - lightDir;
        vec3 eyeDir = normalize(-vPosition);        
        vec4 texColor = texture(uTex0, vTexcoord) * vec4(uColor, 1.0);
        if(dot(lightDir, vNormal) > 0.0f)
        {
            outColor = vec4((dot(reflectDir, eyeDir) * texColor.xyz), 1.0);
        }else
        {
            outColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        }
    }
);

const char* specTexSpotFragSrc = GLSL(
    uniform vec3 uColor;
    uniform sampler2D uTex0;

    layout (std140) uniform UniformBlock
    {
                            // Base alignment    Aligned offset
        mat4 projMat;       // 16 (x4)           0
        vec3 light1;        // 16                64
    };                
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;

    out vec4 outColor;

    void main()
    {

        vec3 lightDir = normalize(light1 - vPosition);
        vec3 reflectDir = 2*dot(lightDir, vNormal)*vNormal - lightDir;
        vec3 eyeDir = normalize(-vPosition);        
        vec4 texColor = texture(uTex0, vTexcoord) * vec4(uColor, 1.0);
        if(dot(lightDir, vNormal) > 0.0f)
        {
            outColor = vec4((dot(reflectDir, eyeDir) * texColor.xyz), 1.0);
        }else
        {
            outColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        }
    }
);

// Fragment shader with phong lighting
const char* ADSFragSrc = GLSL(
    uniform vec3 uColor;
    uniform sampler2D uTex0;

    layout (std140) uniform UniformBlock
    {
                            // Base alignment    Aligned offset
        mat4 projMat;       // 16 (x4)           0
        vec3 light1;        // 16                64
    };                
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;

    out vec4 outColor;

    void main()
    {
        
        vec3 lightDir = normalize(light1 - vPosition);
        vec3 reflectDir = 2*dot(lightDir, vNormal)*vNormal - lightDir;
        //vec3 reflectDir = 2*max(dot(lightDir, vNormal), 0)*vNormal - lightDir;
        vec3 eyeDir = normalize(-vPosition);        
        vec4 texColor = texture(uTex0, vTexcoord) * vec4(uColor, 1.0);

        vec3 ambContrib = 0.1 * texColor.xyz;

        vec3 diffContrib = max(dot(lightDir, vNormal), 0) * texColor.xyz;

        vec3 specContrib = pow(max(dot(reflectDir, eyeDir), 0), 6) * texColor.xyz;
        outColor = vec4(ambContrib + diffContrib + specContrib, 1.0);
    }
);

// Fragment shader with phong lighting and material
const char* OBJFragSrc = GLSL(
    uniform sampler2D uTex0;
    //uniform sampler2D uTex1;

    layout (std140) uniform UniformBlock
    {
                            // Base alignment    Aligned offset
        mat4 projMat;       // 16 (x4)           0
        vec3 light1;        // 16                64
    };                
    
    uniform vec3 Ka;
    uniform vec3 Kd;
    uniform vec3 Ks;
    uniform float Ns;
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;

    out vec4 outColor;

    void main()
    {
        
        vec3 lightDir = normalize(light1 - vPosition);
        vec3 reflectDir = 2*max(dot(lightDir, vNormal), 0)*vNormal - lightDir;
        vec3 eyeDir = normalize(-vPosition);        
        vec4 texColor = texture(uTex0, vTexcoord);

        vec3 ambContrib = Ka * texColor.xyz;

        vec3 diffContrib = max(dot(lightDir, vNormal), 0.0) * texColor.xyz * Kd;

        vec3 specContrib = pow(max(dot(reflectDir, eyeDir), 0), Ns) * texColor.xyz * Ks;
        outColor = vec4(ambContrib + diffContrib + specContrib, 1.0);
    }
);

// Spotlight fragment shader with phong lighting
const char* ADSSpotFragSrc = GLSL(
    uniform vec3 uLightTarget;
    uniform vec3 uColor;
    uniform sampler2D uTex0;

    layout (std140) uniform UniformBlock
    {
                            // Base alignment    Aligned offset
        mat4 projMat;       // 16 (x4)           0
        vec3 light1;        // 16                64
    };                
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;

    out vec4 outColor;

    void main()
    {
        
        vec3 lightDir = normalize(light1 - vPosition);
        vec3 reflectDir = 2*dot(lightDir, vNormal)*vNormal - lightDir;
        vec3 eyeDir = normalize(-vPosition);        
        vec4 texColor = texture(uTex0, vTexcoord) * vec4(uColor, 1.0);

        vec3 ambContrib = 0.1 * texColor.xyz;

        vec3 diffContrib = dot(lightDir, vNormal) * texColor.xyz;

        vec3 specContrib = pow(max(dot(reflectDir, eyeDir), 0), 6) * texColor.xyz;

        vec3 lightToTarget = normalize(light1 - uLightTarget);

        float angleDeg = 30.0;
        vec3 posToLight = normalize(light1 - vPosition);


        if(dot(posToLight, lightToTarget) > cos(angleDeg/180.0*3.14159))
        {
            outColor = vec4(ambContrib + diffContrib + specContrib, 1.0);
        }else
        {
            outColor = vec4(ambContrib, 1.0);
        }
    }
);

// Fragment shader with specular lighting
const char* specularFragSrc = GLSL(
    uniform vec3 uColor;
    layout (std140) uniform UniformBlock
    {
                            // Base alignment    Aligned offset
        mat4 projMat;       // 16 (x4)           0
        vec3 light1;        // 16                64
    };                
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec3 vTexcoord;

    out vec4 outColor;

    void main()
    {
        vec3 lightDir = normalize(light1 - vPosition);
        vec3 reflectDir = 2*dot(lightDir, vNormal)*vNormal - lightDir;
        vec3 eyeDir = normalize(-vPosition);
        outColor = vec4((dot(reflectDir, eyeDir) * uColor), 1.0);
    }
);

// Normal mapping fragment shader
const char* normalFragSrc = GLSL(
    uniform vec3 uColor;
    uniform sampler2D uTex0;
    uniform sampler2D uTex1;   // normal map

    in vec3 vLightT;
    in vec3 vEyeT;
    in vec2 vTexcoord;

    out vec4 outColor;

    void main()
    {

        
        vec4 texColor = texture(uTex0, vTexcoord) * vec4(uColor, 1.0);
        
        vec3 ambContrib = 0.1 * texColor.xyz;
        
        vec3 lightT = normalize(vLightT);

        vec3 normal = (texture(uTex1, vTexcoord)).xyz;
        
        float intensity = dot(normal, lightT);
        vec3 diffContrib = max(dot(normal, lightT), 0) * texColor.xyz;

        vec3 eyeT = normalize(vEyeT);
        vec3 reflectDir = 2*dot(lightT, normal)*normal - lightT;
        vec3 specContrib = pow(max(dot(reflectDir, eyeT), 0.0), 4.0) * texColor.xyz;
        
        outColor = vec4(ambContrib + diffContrib + specContrib, 1.0);
    }
);

//------------------ Post processing fragment shaders

// Inverts the color of the image
const char* invertFragSrc = GLSL(
    uniform sampler2D uTex0;
    
    in vec2 vTexcoord;

    out vec4 outColor;
    
    void main()
    {
        outColor = vec4(vec3(1.0 - texture(uTex0, vTexcoord)), 1.0);
    }
);

// Turns the image to grey scale
const char* greyScaleFragSrc = GLSL(
    uniform sampler2D uTex0;
    
    in vec2 vTexcoord;
    out vec4 outColor;
    
    void main()
    {
        outColor = texture(uTex0, vTexcoord);
        float average = 0.2126 * outColor.r + 0.7152 * outColor.g + 0.0722 * outColor.b;
        outColor = vec4(average, average, average, 1.0);
    }
);

// Sharpens the edges in the scene
const char* sharpenFragSrc = GLSL(
    uniform sampler2D uTex0;
    
    in vec2 vTexcoord;

    out vec4 outColor;

    const float offset = 1.0 / 300;
    
    void main()
    {
        vec2 offsets[9] = vec2[](
            vec2(-offset, offset),
            vec2(0.0f, offset),
            vec2(offset, offset),
            vec2(-offset, 0.0f),
            vec2(0.0f, 0.0f),
            vec2(offset, 0.0f),
            vec2(-offset, -offset),
            vec2(0.0f, -offset),
            vec2(offset, -offset)         
        );

        float kernel[9] = float[](
            -1, -1, -1,
            -1, 9, -1,
            -1, -1, -1
        );

        vec3 sampleTex[9];
        for(int i = 0; i < 9; i++)
        {
            sampleTex[i] = vec3(texture(uTex0, vTexcoord.st + offsets[i]));
        }

        vec3 col;
        for(int i = 0; i < 9; i++)
            col += sampleTex[i] * kernel[i];

        outColor = vec4(col, 1.0);
    }
);

// Blurs everything
const char* blurFragSrc = GLSL(
    uniform sampler2D uTex0;
    
    in vec2 vTexcoord;

    out vec4 outColor;

    const float offset = 1.0 / 300;
    
    void main()
    {
        vec2 offsets[9] = vec2[](
            vec2(-offset, offset),
            vec2(0.0f, offset),
            vec2(offset, offset),
            vec2(-offset, 0.0f),
            vec2(0.0f, 0.0f),
            vec2(offset, 0.0f),
            vec2(-offset, -offset),
            vec2(0.0f, -offset),
            vec2(offset, -offset)         
        );

        float kernel[9] = float[](
            1.0/16, 2.0/16, 1.0/16,
            2.0/16, 4.0/16, 2/16,
            1.0/16, 2.0/16, 1.0/16
        );

        vec3 sampleTex[9];
        for(int i = 0; i < 9; i++)
        {
            sampleTex[i] = vec3(texture(uTex0, vTexcoord.st + offsets[i]));
        }

        vec3 col;
        for(int i = 0; i < 9; i++)
            col += sampleTex[i] * kernel[i];

        outColor = vec4(col, 1.0);
    }
);

// Brighten the edges and darken everything else
const char* brightEdgeFragSrc = GLSL(
    uniform sampler2D uTex0;
    
    in vec2 vTexcoord;

    out vec4 outColor;

    const float offset = 1.0 / 300;
    
    void main()
    {
        vec2 offsets[9] = vec2[](
            vec2(-offset, offset),
            vec2(0.0f, offset),
            vec2(offset, offset),
            vec2(-offset, 0.0f),
            vec2(0.0f, 0.0f),
            vec2(offset, 0.0f),
            vec2(-offset, -offset),
            vec2(0.0f, -offset),
            vec2(offset, -offset)         
        );

        float kernel[9] = float[](
            1, 1, 1,
            1, -8, 1,
            1, 1, 1
        );

        vec3 sampleTex[9];
        for(int i = 0; i < 9; i++)
        {
            sampleTex[i] = vec3(texture(uTex0, vTexcoord.st + offsets[i]));
        }

        vec3 col;
        for(int i = 0; i < 9; i++)
            col += sampleTex[i] * kernel[i];

        outColor = vec4(col, 1.0);
    }
);

//--------------------- GEOMETRY SHADERS

const char* exampleGeoSrc = GLSL(
    in VS_OUT {
        vec3 normal;
    } gs_in[];
    
    //layout (points) in;
    layout (triangles) in;
    layout (line_strip, max_vertices = 6) out;

    void GenerateLine(int index)
    {
        gl_Position = gl_in[index].gl_Position;
        EmitVertex();

        gl_Position = gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0f) * 0.4;
        EmitVertex();
        EndPrimitive();
    }
    
    void main() {
        GenerateLine(0);
        GenerateLine(1);
        GenerateLine(2);    
    }
);

#endif
