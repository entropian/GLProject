#ifndef SHADERS_H
#define SHADERS_H

#include <GL/glew.h>


#define GLSL(src) "#version 150 core\n" #src
//////////////// Shaders
//////// Vertex Shaders
const char* basicVertSrc = GLSL(
    uniform mat4 uModelViewMat;
    uniform mat4 uNormalMat;
    uniform mat4 uProjMat;

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
        //vTexcoord = vec2(aTexcoord.x, 1 - aTexcoord.y);
        vTexcoord = aTexcoord;
        gl_Position = uProjMat * vec4(vPosition, 1.0);
    }
);

const char* normalVertSrc = GLSL(
    uniform mat4 uModelViewMat;
    uniform mat4 uNormalMat;
    uniform mat4 uProjMat;
    uniform vec3 uLight;

    in vec3 aPosition;
    in vec3 aNormal;
    in vec2 aTexcoord;

    in vec3 aTangent;
    in vec3 aBinormal;
    in float aDet;
    
    //out vec3 vPosition;
    out vec3 vLightT;
    out vec3 vEyeT;
    out vec3 vLightTarget;
    out vec2 vTexcoord;
    
    void main()
    {
        vTexcoord = aTexcoord;
        mat4 inverseMat = inverse(uModelViewMat);
        vec3 lightM = (inverseMat * vec4(uLight, 1.0)).xyz - aPosition;
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

        gl_Position = uProjMat * uModelViewMat * vec4(aPosition, 1.0);
    }
);

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

const char* diffuseVertSrc = GLSL(
    uniform mat4 uModelViewMat;
    uniform mat4 uNormalMat;
    uniform mat4 uProjMat;
    uniform vec3 uColor;
    // uLight is in eye space
    uniform vec3 uLight;

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
        vec3 lightDirE = normalize(uLight - posE);

        vColor = uColor * max(dot(lightDirE, normE), 0.0);

        vTexcoord = aTexcoord;
        gl_Position = uProjMat * uModelViewMat * vec4(aPosition, 1.0);
    }
);

// slightly better shader with lighting that accounts for the camera position
const char* lightVertexSrc = GLSL(
    uniform mat4 uModelViewMat;
    uniform mat4 uNormalMat;
    uniform mat4 uProjMat;
    uniform vec3 uColor;
    // uLight is in eye space
    uniform vec3 uLight;

    in vec3 aPosition;
    in vec3 aNormal;
    in vec2 aTexcoord;

    out vec3 vColor;
    out vec2 vTexcoord;
        
    void main() {
        // in eye space
        vec3 posE = (uModelViewMat * vec4(aPosition, 1.0)).xyz;
        vec3 normE = (uNormalMat * vec4(aNormal, 1.0)).xyz;
        vec3 lightDirE = normalize(uLight - posE);
        vec3 eyeDirE = normalize(-posE);
        vec3 reflectDirE = 2.0*dot(lightDirE, normE)*normE - lightDirE;

        vColor = uColor * pow(max(dot(eyeDirE, reflectDirE), 0.0), 10);
        
        vTexcoord = aTexcoord;
        gl_Position = uProjMat * uModelViewMat * vec4(aPosition, 1.0);
    }
);

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


//////// Fragment Shaders
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

const char* diffuseFragSrc = GLSL(
    uniform vec3 uLight;
    uniform vec3 uColor;
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec3 vTexcoord;

    out vec4 outColor;

    void main()
    {
        vec3 lightDir = normalize(uLight - vPosition);
        vec3 normal = normalize(vNormal);
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

const char* basicFragSrc = GLSL(
    uniform vec3 uLight;
    uniform vec3 uColor;
    uniform sampler2D uTex0;
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;

    out vec4 outColor;

    void main()
    {

        vec3 lightDir = normalize(uLight - vPosition);
        vec3 normal = normalize(vNormal);
        vec4 texColor = texture(uTex0, vTexcoord) * vec4(uColor, 1.0);
        outColor = vec4((dot(lightDir, normal) * texColor.xyz), 1.0);
    }
);

// Looks weird with really hard shadow edges
const char* specTexFragSrc = GLSL(
    uniform vec3 uLight;
    uniform vec3 uColor;
    uniform sampler2D uTex0;
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;

    out vec4 outColor;

    void main()
    {

        vec3 lightDir = normalize(uLight - vPosition);
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
    uniform vec3 uLight;
    uniform vec3 uColor;
    uniform sampler2D uTex0;
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;

    out vec4 outColor;

    void main()
    {

        vec3 lightDir = normalize(uLight - vPosition);
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

const char* ADSFragSrc = GLSL(
    uniform vec3 uLight;
    uniform vec3 uColor;
    uniform sampler2D uTex0;
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;

    out vec4 outColor;

    void main()
    {
        
        vec3 lightDir = normalize(uLight - vPosition);
        vec3 reflectDir = 2*dot(lightDir, vNormal)*vNormal - lightDir;
        vec3 eyeDir = normalize(-vPosition);        
        vec4 texColor = texture(uTex0, vTexcoord) * vec4(uColor, 1.0);

        vec3 ambContrib = 0.1 * texColor.xyz;

        vec3 diffContrib = dot(lightDir, vNormal) * texColor.xyz;

        vec3 specContrib = pow(max(dot(reflectDir, eyeDir), 0), 6) * texColor.xyz;
        outColor = vec4(ambContrib + diffContrib + specContrib, 1.0);
    }
);

const char* OBJFragSrc = GLSL(
    uniform vec3 uLight;
    uniform sampler2D uTex0;
    //uniform sampler2D uTex1;

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
        
        vec3 lightDir = normalize(uLight - vPosition);
        vec3 reflectDir = 2*dot(lightDir, vNormal)*vNormal - lightDir;
        vec3 eyeDir = normalize(-vPosition);        
        vec4 texColor = texture(uTex0, vTexcoord);

        vec3 ambContrib = Ka * texColor.xyz;

        vec3 diffContrib = max(dot(lightDir, vNormal), 0.0) * texColor.xyz * Kd;

        vec3 specContrib = pow(max(dot(reflectDir, eyeDir), 0), Ns) * texColor.xyz * Ks;
        outColor = vec4(ambContrib + diffContrib + specContrib, 1.0);
    }
);
const char* ADSSpotFragSrc = GLSL(
    uniform vec3 uLight;
    uniform vec3 uLightTarget;
    uniform vec3 uColor;
    uniform sampler2D uTex0;
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;


    out vec4 outColor;

    void main()
    {
        
        vec3 lightDir = normalize(uLight - vPosition);
        vec3 reflectDir = 2*dot(lightDir, vNormal)*vNormal - lightDir;
        vec3 eyeDir = normalize(-vPosition);        
        vec4 texColor = texture(uTex0, vTexcoord) * vec4(uColor, 1.0);

        vec3 ambContrib = 0.1 * texColor.xyz;

        vec3 diffContrib = dot(lightDir, vNormal) * texColor.xyz;

        vec3 specContrib = pow(max(dot(reflectDir, eyeDir), 0), 6) * texColor.xyz;

        vec3 lightToTarget = normalize(uLight - uLightTarget);

        float angleDeg = 30.0;
        vec3 posToLight = normalize(uLight - vPosition);


        if(dot(posToLight, lightToTarget) > cos(angleDeg/180.0*3.14159))
        {
            outColor = vec4(ambContrib + diffContrib + specContrib, 1.0);
        }else
        {
            outColor = vec4(ambContrib, 1.0);
        }
    }
);


const char* specularFragSrc = GLSL(
    uniform vec3 uLight;
    uniform vec3 uColor;
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec3 vTexcoord;

    out vec4 outColor;

    void main()
    {
        vec3 lightDir = normalize(uLight - vPosition);
        vec3 reflectDir = 2*dot(lightDir, vNormal)*vNormal - lightDir;
        vec3 eyeDir = normalize(-vPosition);
        outColor = vec4((dot(reflectDir, eyeDir) * uColor), 1.0);
    }
);



#endif
