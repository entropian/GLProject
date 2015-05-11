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
        vTexcoord = aTexcoord;
        gl_Position = uProjMat * vec4(vPosition, 1.0);
    }
);



// TODO: why doesn't this work anymore?
// the most primitive shader with lighting
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

        if(dot(lightDirE, normE) > 0)
        {
            vColor = uColor * dot(lightDirE, normE);
        }else
        {
            vColor = vec3(0, 0, 0);
        }

        vTexcoord = aTexcoord;

        // old
        // gl_position = proj * view * trans * vec4(position, 1.0);
        // TODO: switch in vPosition to reduce the number of calculations
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

        if(dot(eyeDirE, reflectDirE) > 0)
        {
            vColor = uColor * dot(eyeDirE, reflectDirE);
        }else
        {
            vColor = vec3(0, 0, 0);
        }

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
        outColor = vec4((dot(lightDir, vNormal) * uColor), 1.0);
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
        vec4 texColor = texture(uTex0, vTexcoord) * vec4(uColor, 1.0);
        outColor = vec4((dot(lightDir, vNormal) * texColor.xyz), 1.0);

        //outColor = texture(uTex0, vTexcoord) * vec4(uColor, 1.0);
        //outColor = vec4(uColor, 1.0);
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