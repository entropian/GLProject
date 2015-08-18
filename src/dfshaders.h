#ifndef DFSHADERS_H
#define DFSHADERS_H

#include <GL/glew.h>

#define GLSL(src) "#version 330 core\n\
layout (std140) uniform UniformBlock{\
mat4 projMat;\
vec3 light1;\
mat4 lightSpaceMat;\
vec3 eyeW;\
};\
const float NEAR = 0.1f;\
const float FAR = 50.0f;\
float LinearizeDepth(float depth)\
{\
    float z = depth * 2.0 - 1.0;\
    return (2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));\
}\n" #src


const char* GeoPassBasicVertSrc = GLSL(    
    uniform mat4 uModelViewMat;
    uniform mat4 uNormalMat;

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

const char* GeoPassNormalVertSrc = GLSL(    
    uniform mat4 uModelViewMat;
    uniform mat4 uNormalMat;

    in vec3 aPosition;
    in vec3 aNormal;
    in vec2 aTexcoord;

    in vec3 aTangent;
    in vec3 aBinormal;
    in float aDet;

    out vec3 vPosition;    
    out vec2 vTexcoord;
    out mat3 vTBNViewMat;
    
    void main()
    {
        vPosition = (uModelViewMat * vec4(aPosition, 1.0)).xyz;
        vTexcoord = aTexcoord;
        vec3 T = normalize(vec3(uModelViewMat * vec4(aTangent, 0.0)));
        vec3 B = normalize(vec3(uModelViewMat * vec4(aBinormal * aDet, 0.0)));
        vec3 N = normalize(vec3(uModelViewMat * vec4(aNormal, 0.0)));

        vTBNViewMat = mat3(T, B, N);

        gl_Position = projMat * vec4(vPosition, 1.0);
    }
);


const char* GeoPassOBJDFragSrc = GLSL(
    uniform sampler2D diffuseMap;

    uniform vec3 Ka;
    uniform vec3 Kd;
    uniform vec3 Ks;
    uniform float Ns;
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;

    layout (location = 0) out vec3 gPositionDepth;
    layout (location = 1) out vec4 gNormalSpec;
    layout (location = 2) out vec4 gDiffuse;

    void main()
    {
        // world space
        gPositionDepth.xyz = vPosition;
        gNormalSpec.xyz = vNormal;
        gNormalSpec.a = Ns;
        gDiffuse.rgb = texture(diffuseMap, vTexcoord).rgb * Kd;
        gDiffuse.a = 1.0;
    }
);

const char* GeoPassOBJDSFragSrc = GLSL(
    uniform sampler2D diffuseMap;
    uniform sampler2D specularMap;

    uniform vec3 Ka;
    uniform vec3 Kd;
    uniform vec3 Ks;
    uniform float Ns;
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;

    layout (location = 0) out vec3 gPositionDepth;
    layout (location = 1) out vec4 gNormalSpec;
    layout (location = 2) out vec4 gDiffuse;

    void main()
    {
        // world space
        gPositionDepth.xyz = vPosition;
        gNormalSpec.xyz = vNormal;
        gNormalSpec.a = Ns;
        gDiffuse.rgb = texture(diffuseMap, vTexcoord).rgb * Kd;
        vec3 specular = texture(specularMap, vTexcoord).rgb;
        gDiffuse.a = (specular.r + specular.g + specular.b) / 3.0;
    }
);

const char* GeoPassOBJADSFragSrc = GLSL(
    uniform sampler2D diffuseMap;
    uniform sampler2D specularMap;
    uniform sampler2D alphaMap;

    uniform vec3 Ka;
    uniform vec3 Kd;
    uniform vec3 Ks;
    uniform float Ns;
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;

    layout (location = 0) out vec3 gPositionDepth;
    layout (location = 1) out vec4 gNormalSpec;
    layout (location = 2) out vec4 gDiffuse;

    void main()
    {
        // world space
        float alpha = texture(alphaMap, vTexcoord).r;
        if(alpha < 0.1)
            discard;
        gPositionDepth.xyz = vPosition;
        gNormalSpec.xyz = vNormal;
        gNormalSpec.a = Ns;
        gDiffuse.rgb = texture(diffuseMap, vTexcoord).rgb * Kd;
        vec3 specular = texture(specularMap, vTexcoord).rgb;
        gDiffuse.a = (specular.r + specular.g + specular.b) / 3.0;
    }
);

const char* GeoPassOBJNDFragSrc = GLSL(
    uniform sampler2D diffuseMap;
    uniform sampler2D normalMap;

    uniform vec3 Ka;
    uniform vec3 Kd;
    uniform vec3 Ks;
    uniform float Ns;
    
    in vec3 vPosition;
    in vec2 vTexcoord;
    in mat3 vTBNViewMat;

    layout (location = 0) out vec3 gPositionDepth;
    layout (location = 1) out vec4 gNormalSpec;
    layout (location = 2) out vec4 gDiffuse;

    void main()
    {
        // world space
        gPositionDepth.xyz = vPosition;
        vec3 normal = texture(normalMap, vTexcoord).xyz;
        normal = vTBNViewMat * normal;
        gNormalSpec.xyz = normal;
        gNormalSpec.a = Ns;
        gDiffuse.rgb = texture(diffuseMap, vTexcoord).rgb * Kd;
        gDiffuse.a = 1.0;
    }
);

const char* GeoPassOBJNADFragSrc = GLSL(
    uniform sampler2D diffuseMap;
    uniform sampler2D normalMap;
    uniform sampler2D alphaMap;

    uniform vec3 Ka;
    uniform vec3 Kd;
    uniform vec3 Ks;
    uniform float Ns;
    
    in vec3 vPosition;
    in vec2 vTexcoord;
    in mat3 vTBNViewMat;

    layout (location = 0) out vec3 gPositionDepth;
    layout (location = 1) out vec4 gNormalSpec;
    layout (location = 2) out vec4 gDiffuse;

    void main()
    {
        // world space
        float alpha = texture(alphaMap, vTexcoord).r;
        if(alpha < 0.1)
            discard;
        gPositionDepth.xyz = vPosition;
        vec3 normal = texture(normalMap, vTexcoord).xyz;
        normal = vTBNViewMat * normal;
        gNormalSpec.xyz = normal;
        gNormalSpec.a = Ns;
        gDiffuse.rgb = texture(diffuseMap, vTexcoord).rgb * Kd;
        gDiffuse.a = 1.0;
    }
);

const char* GeoPassOBJNDSFragSrc = GLSL(
    uniform sampler2D diffuseMap;
    uniform sampler2D normalMap;
    uniform sampler2D specularMap;

    uniform vec3 Ka;
    uniform vec3 Kd;
    uniform vec3 Ks;
    uniform float Ns;
    
    in vec3 vPosition;
    in vec2 vTexcoord;
    in mat3 vTBNViewMat;

    layout (location = 0) out vec3 gPositionDepth;
    layout (location = 1) out vec4 gNormalSpec;
    layout (location = 2) out vec4 gDiffuse;

    void main()
    {
        // world space
        gPositionDepth.xyz = vPosition;
        vec3 normal = texture(normalMap, vTexcoord).xyz;
        normal = vTBNViewMat * normal;
        gNormalSpec.xyz = normal;
        gNormalSpec.a = Ns;
        gDiffuse.rgb = texture(diffuseMap, vTexcoord).rgb * Kd;
        vec3 specular = texture(specularMap, vTexcoord).rgb;
        gDiffuse.a = (specular.r + specular.g + specular.b) / 3.0;        
    }
);

const char* GeoPassOBJNADSFragSrc = GLSL(
    uniform sampler2D diffuseMap;
    uniform sampler2D normalMap;
    uniform sampler2D specularMap;
    uniform sampler2D alphaMap;

    uniform vec3 Ka;
    uniform vec3 Kd;
    uniform vec3 Ks;
    uniform float Ns;
    
    in vec3 vPosition;
    in vec2 vTexcoord;
    in mat3 vTBNViewMat;

    layout (location = 0) out vec3 gPositionDepth;
    layout (location = 1) out vec4 gNormalSpec;
    layout (location = 2) out vec4 gDiffuse;

    void main()
    {
        // world space
        float alpha = texture(alphaMap, vTexcoord).r;
        if(alpha < 0.1)
            discard;
        gPositionDepth.xyz = vPosition;
        vec3 normal = texture(normalMap, vTexcoord).xyz;
        normal = vTBNViewMat * normal;
        gNormalSpec.xyz = normal;
        gNormalSpec.a = Ns;
        gDiffuse.rgb = texture(diffuseMap, vTexcoord).rgb * Kd;
        vec3 specular = texture(specularMap, vTexcoord).rgb;
        gDiffuse.a = (specular.r + specular.g + specular.b) / 3.0;        
    }
);


const char* LightPassFragSrc = GLSL(
    uniform sampler2D gDiffuse;
    uniform sampler2D gNormalSpec;
    uniform sampler2D gPositionDepth;

    in vec2 vTexcoord;
    out vec4 outColor;

    void main()
    {
        vec3 posE = texture(gPositionDepth, vTexcoord).rgb;
        vec3 normal = normalize(texture(gNormalSpec, vTexcoord).rgb);
        vec3 diffuse = texture(gDiffuse, vTexcoord).rgb;
        float specExponent = texture(gNormalSpec, vTexcoord).a;
        float specIntensity = texture(gDiffuse, vTexcoord).a;
        
        vec3 ambContrib = diffuse * 0.3;
        vec3 lightDir = normalize(light1 - posE);
        vec3 eyeDir = normalize(-posE);
        vec3 reflectDir = 2*dot(normal, lightDir)*normal - lightDir;

        float intensity = max(dot(normal, lightDir), 0);
        vec3 diffContrib = intensity * diffuse;
        vec3 specContrib = pow(max(dot(eyeDir, reflectDir), 0), specExponent) * diffuse * specIntensity;
        outColor = vec4(ambContrib + diffContrib + specContrib, 1.0);
    }
);

const char* shLightPassFragSrc = GLSL(
    uniform sampler2D gDiffuse;
    uniform sampler2D gNormalSpec;
    uniform sampler2D gPositionDepth;
    uniform sampler2D shadowMap;

    in vec2 vTexcoord;
    out vec4 outColor;

    float ShadowCalculation(vec4 fragPosLightSpace)
    {
        // perform perspective divide
        vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        
        // Transform to [0,1] range
        projCoords = projCoords * 0.5 + 0.5;
        
        // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
        float closestDepth = texture(shadowMap, projCoords.xy).r;
        
        // Get depth of current fragment from light's perspective
        float currentDepth = projCoords.z;
        
        // Check whether current frag pos is in shadow
        float bias = 0.005;
        //float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;


        float shadow = 0.0;
        vec2 texelSize = 1.0 / (textureSize(gDiffuse, 0) * 10);
        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
            }    
        }
        shadow /= 9.0;

        return shadow;
    }    
    
    void main()
    {
        vec3 posW = texture(gPositionDepth, vTexcoord).rgb;
        vec4 fragPosL = lightSpaceMat * vec4(posW, 1.0);
        vec3 normal = normalize(texture(gNormalSpec, vTexcoord).rgb);
        vec3 diffuse = texture(gDiffuse, vTexcoord).rgb;
        float specExponent = texture(gNormalSpec, vTexcoord).a;
        float specIntensity = texture(gDiffuse, vTexcoord).a;
        
        vec3 ambContrib = diffuse * 0.3;
        vec3 lightDir = normalize(light1 - posW);
        vec3 eyeDir = normalize(eyeW - posW);
        vec3 reflectDir = 2*dot(normal, lightDir)*normal - lightDir;

        float intensity = max(dot(normal, lightDir), 0);
        vec3 diffContrib = intensity * diffuse;
        vec3 specContrib = pow(max(dot(eyeDir, reflectDir), 0), specExponent) * diffuse * specIntensity;
        float shadow = ShadowCalculation(fragPosL);
        vec3 lighting = ambContrib + (1.0 - shadow) * (diffContrib + specContrib);
        outColor = vec4(lighting, 1.0);
        //outColor = vec4(ambContrib + diffContrib + specContrib, 1.0);
    }
);

#endif
