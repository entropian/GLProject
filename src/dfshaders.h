#ifndef DFSHADERS_H
#define DFSHADERS_H

#include <GL/glew.h>

#define GLSL(src) "#version 330 core\n\
layout (std140) uniform UniformBlock{\
mat4 projMat;\
vec3 light1;\
mat4 lightSpaceMat;\
vec3 eyeW;\
};\n" #src


const char* GeoPassVertSrc = GLSL(    
    uniform mat4 uModelMat;
    uniform mat4 uViewMat;
    uniform mat4 uNormalMat;

    in vec3 aPosition;
    in vec3 aNormal;
    in vec2 aTexcoord;

    out vec3 vPosition;
    out vec3 vNormal;
    out vec2 vTexcoord;

    void main()
    {        
        vPosition = (uModelMat * vec4(aPosition, 1.0)).xyz;
        vNormal = (uNormalMat * vec4(aNormal, 1.0)).xyz;
        vTexcoord = aTexcoord;
        gl_Position = projMat * uViewMat * vec4(vPosition, 1.0);
    }
);

const char* GeoPassFragSrc = GLSL(
    uniform sampler2D diffuseMap;

    uniform vec3 Ka;
    uniform vec3 Kd;
    uniform vec3 Ks;
    uniform float Ns;
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;

    layout (location = 0) out vec3 gPosition;
    layout (location = 1) out vec4 gNormalSpec;
    layout (location = 2) out vec3 gDiffuse;

    void main()
    {
        // world space
        gPosition = vPosition;
        gNormalSpec.xyz = vNormal;
        gNormalSpec.a = Ns;
        gDiffuse = texture(diffuseMap, vTexcoord).rgb * Kd;
    }
);

const char* LightPassFragSrc = GLSL(
    uniform sampler2D gDiffuse;
    uniform sampler2D gNormalSpec;
    uniform sampler2D gPosition;

    in vec2 vTexcoord;
    out vec4 outColor;

    void main()
    {
        vec3 posW = texture(gPosition, vTexcoord).rgb;
        vec3 normal = texture(gNormalSpec, vTexcoord).rgb;
        vec3 diffuse = texture(gDiffuse, vTexcoord).rgb;
        float specExponent = texture(gNormalSpec, vTexcoord).a;

        vec3 ambContrib = diffuse * 0.3;
        vec3 lightDir = normalize(light1 - posW);
        vec3 eyeDir = normalize(eyeW - posW);

        float intensity = max(dot(normal, lightDir), 0);
        vec3 diffContrib = intensity * diffuse;
        vec3 specContrib = pow(intensity, specExponent) * diffuse;
        outColor = vec4(ambContrib + diffContrib + specContrib, 1.0);
    }
);

#endif
