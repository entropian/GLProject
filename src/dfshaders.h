#ifndef DFSHADERS_H
#define DFSHADERS_H

#include <GL/glew.h>

#define GLSL(src) "#version 330 core\n\
layout (std140) uniform UniformBlock{\
mat4 projMat;\
vec3 light1;\
mat4 lightSpaceMat;\
mat4 invViewMat;\
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

    layout (location = 0) in vec3 aPosition;
    layout (location = 1) in vec3 aNormal;
    layout (location = 2) in vec2 aTexcoord;

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

    layout (location = 0) in vec3 aPosition;
    layout (location = 1) in vec3 aNormal;
    layout (location = 2) in vec2 aTexcoord;

    layout (location = 3) in vec3 aTangent;
    layout (location = 4) in vec3 aBinormal;
    layout (location = 5) in float aDet;

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

const char* GeoPassBasicFragSrc = GLSL(
    uniform sampler2D diffuseMap;

    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;

    layout (location = 0) out vec4 gPositionDepth;
    layout (location = 1) out vec4 gNormalSpec;
    layout (location = 2) out vec4 gDiffuse;

    void main()
    {
        // world space
        gPositionDepth.xyz = vPosition;
        gPositionDepth.a = LinearizeDepth(gl_FragCoord.z);
        gNormalSpec.xyz = vNormal;
        gNormalSpec.a = 40;
        gDiffuse.rgb = texture(diffuseMap, vTexcoord).rgb;
        gDiffuse.a = 1.0;
    }
);

const char* GeoPassNormalFragSrc = GLSL(
    uniform sampler2D diffuseMap;
    uniform sampler2D normalMap;

    in vec3 vPosition;
    in vec2 vTexcoord;
    in mat3 vTBNViewMat;

    layout (location = 0) out vec4 gPositionDepth;
    layout (location = 1) out vec4 gNormalSpec;
    layout (location = 2) out vec4 gDiffuse;

    void main()
    {
        // world space
        gPositionDepth.xyz = vPosition;
        gPositionDepth.a = LinearizeDepth(gl_FragCoord.z);        
        vec3 normal = texture(normalMap, vTexcoord).xyz;
        normal = vTBNViewMat * normal;
        gNormalSpec.xyz = normal;        
        gNormalSpec.a = 40;
        gDiffuse.rgb = texture(diffuseMap, vTexcoord).rgb;
        gDiffuse.a = 1.0;
    }
);

const char* GeoPassOBJFragSrc = GLSL(
    uniform vec3 Ka;
    uniform vec3 Kd;
    uniform vec3 Ks;
    uniform float Ns;
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;

    layout (location = 0) out vec4 gPositionDepth;
    layout (location = 1) out vec4 gNormalSpec;
    layout (location = 2) out vec4 gDiffuse;

    void main()
    {
        // world space
        gPositionDepth.xyz = vPosition;
        gPositionDepth.a = LinearizeDepth(gl_FragCoord.z);
        gNormalSpec.xyz = vNormal;
        gNormalSpec.a = Ns;
        gDiffuse.rgb = Kd;
        //gDiffuse.a = 1.0;
        gDiffuse.a = Ns / 255.0;
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

    layout (location = 0) out vec4 gPositionDepth;
    layout (location = 1) out vec4 gNormalSpec;
    layout (location = 2) out vec4 gDiffuse;

    void main()
    {
        // world space
        gPositionDepth.xyz = vPosition;
        gPositionDepth.a = LinearizeDepth(gl_FragCoord.z);
        gNormalSpec.xyz = vNormal;
        gNormalSpec.a = Ns;
        gDiffuse.rgb = texture(diffuseMap, vTexcoord).rgb;
        //gDiffuse.a = 1.0;
        gDiffuse.a = Ns / 255.0;
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

    layout (location = 0) out vec4 gPositionDepth;
    layout (location = 1) out vec4 gNormalSpec;
    layout (location = 2) out vec4 gDiffuse;

    void main()
    {
        // world space
        gPositionDepth.xyz = vPosition;
        gPositionDepth.a = LinearizeDepth(gl_FragCoord.z);        
        gNormalSpec.xyz = vNormal;
        gNormalSpec.a = Ns;
        gDiffuse.rgb = texture(diffuseMap, vTexcoord).rgb;
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

    layout (location = 0) out vec4 gPositionDepth;
    layout (location = 1) out vec4 gNormalSpec;
    layout (location = 2) out vec4 gDiffuse;

    void main()
    {
        // world space
        float alpha = texture(alphaMap, vTexcoord).r;
        if(alpha < 0.1)
            discard;
        gPositionDepth.xyz = vPosition;
        gPositionDepth.a = LinearizeDepth(gl_FragCoord.z);        
        gNormalSpec.xyz = vNormal;
        gNormalSpec.a = Ns;
        gDiffuse.rgb = texture(diffuseMap, vTexcoord).rgb;
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

    layout (location = 0) out vec4 gPositionDepth;
    layout (location = 1) out vec4 gNormalSpec;
    layout (location = 2) out vec4 gDiffuse;

    void main()
    {
        // world space
        gPositionDepth.xyz = vPosition;
        gPositionDepth.a = LinearizeDepth(gl_FragCoord.z);        
        vec3 normal = texture(normalMap, vTexcoord).xyz;
        normal = vTBNViewMat * normal;
        gNormalSpec.xyz = normal;        
        gNormalSpec.a = Ns;
        gDiffuse.rgb = texture(diffuseMap, vTexcoord).rgb;
        //gDiffuse.a = 1.0;
        gDiffuse.a = Ns / 255.0;
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

    layout (location = 0) out vec4 gPositionDepth;
    layout (location = 1) out vec4 gNormalSpec;
    layout (location = 2) out vec4 gDiffuse;

    void main()
    {
        // world space
        float alpha = texture(alphaMap, vTexcoord).r;
        if(alpha < 0.1)
            discard;
        gPositionDepth.xyz = vPosition;
        gPositionDepth.a = LinearizeDepth(gl_FragCoord.z);        
        vec3 normal = texture(normalMap, vTexcoord).xyz;
        normal = vTBNViewMat * normal;
        gNormalSpec.xyz = normal;
        gNormalSpec.a = Ns;
        gDiffuse.rgb = texture(diffuseMap, vTexcoord).rgb;
        //gDiffuse.a = 1.0;
        gDiffuse.a = Ns / 255.0;        
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

    layout (location = 0) out vec4 gPositionDepth;
    layout (location = 1) out vec4 gNormalSpec;
    layout (location = 2) out vec4 gDiffuse;

    void main()
    {
        // world space
        gPositionDepth.xyz = vPosition;
        gPositionDepth.a = LinearizeDepth(gl_FragCoord.z);        
        vec3 normal = texture(normalMap, vTexcoord).xyz;
        normal = vTBNViewMat * normal;
        gNormalSpec.xyz = normal;
        gNormalSpec.a = Ns;
        gDiffuse.rgb = texture(diffuseMap, vTexcoord).rgb;
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

    layout (location = 0) out vec4 gPositionDepth;
    layout (location = 1) out vec4 gNormalSpec;
    layout (location = 2) out vec4 gDiffuse;

    void main()
    {
        // world space
        float alpha = texture(alphaMap, vTexcoord).r;
        if(alpha < 0.1)
            discard;
        gPositionDepth.xyz = vPosition;
        gPositionDepth.a = LinearizeDepth(gl_FragCoord.z);        
        vec3 normal = texture(normalMap, vTexcoord).xyz;
        normal = vTBNViewMat * normal;
        gNormalSpec.xyz = normal;
        gNormalSpec.a = Ns;
        gDiffuse.rgb = texture(diffuseMap, vTexcoord).rgb;
        vec3 specular = texture(specularMap, vTexcoord).rgb;
        gDiffuse.a = (specular.r + specular.g + specular.b) / 3.0;        
    }
);

/*
const char* LightPassFragSrc = GLSL(
    uniform sampler2D gDiffuse;
    uniform sampler2D gNormalSpec;
    uniform sampler2D gPositionDepth;
    uniform sampler2D ssao;

    in vec2 vTexcoord;
    out vec4 outColor;

    void main()
    {
        vec3 posE = texture(gPositionDepth, vTexcoord).rgb;
        vec3 normal = normalize(texture(gNormalSpec, vTexcoord).rgb);
        vec3 diffuse = texture(gDiffuse, vTexcoord).rgb;
        float specExponent = texture(gNormalSpec, vTexcoord).a;
        float specIntensity = texture(gDiffuse, vTexcoord).a;
        float occlusion = texture(ssao, vTexcoord).r;
        vec3 lightColor = vec3(500.0);

        
        vec3 ambContrib = diffuse * 0.3 * occlusion;
        vec3 lightDist = light1 - posE;
        float attenuation = 1/dot(lightDist, lightDist);        
        vec3 lightDir = normalize(lightDist);
        vec3 eyeDir = normalize(-posE);
        vec3 reflectDir = 2*dot(normal, lightDir)*normal - lightDir;

        float intensity = max(dot(normal, lightDir), 0);
        vec3 diffContrib = intensity * diffuse;
        vec3 specContrib = pow(max(dot(eyeDir, reflectDir), 0), specExponent) * diffuse * specIntensity;
        outColor = vec4(ambContrib + (diffContrib + specContrib) * lightColor * attenuation, 1.0);
    }
);
*/


// Phong lighting without normalization
const char* LightPassFragSrc = GLSL(
    uniform sampler2D gDiffuse;
    uniform sampler2D gNormalSpec;
    uniform sampler2D gPositionDepth;
    uniform sampler2D ssao;

    in vec2 vTexcoord;
    layout (location = 0) out vec4 outColor;
    layout (location = 1) out vec4 brightColor;    

    void main()
    {
        vec3 posE = texture(gPositionDepth, vTexcoord).rgb;
        vec3 normal = normalize(texture(gNormalSpec, vTexcoord).rgb);
        vec3 diffuse = texture(gDiffuse, vTexcoord).rgb;
        //float specExponent = texture(gNormalSpec, vTexcoord).a;
        //float specIntensity = texture(gDiffuse, vTexcoord).a;
        float specExponent = texture(gDiffuse, vTexcoord).a * 255.0;
        float occlusion = texture(ssao, vTexcoord).r;
        vec3 lightColor = vec3(100.0);

        
        vec3 ambContrib = diffuse * 0.3 * occlusion;
        vec3 lightDist = light1 - posE;
        float attenuation = 1/dot(lightDist, lightDist);        
        vec3 lightDir = normalize(lightDist);
        vec3 eyeDir = normalize(-posE);
        float intensity = dot(normal, lightDir);        
        //vec3 reflectDir = 2*intensity*normal - lightDir;
        vec3 tmpHalfway = lightDir + eyeDir;
        //vec3 halfwayDir = normalize((lightDir + eyeDir) * clamp(dot(normal, lightDir), 0, 1));
        vec3 halfwayDir = normalize(tmpHalfway * clamp(intensity, 0, 1));
        

        //float normalFactor = (specExponent + 2) * 0.5;
        float normalFactor = (specExponent + 8) * 0.125;
        //vec3 diffContrib = intensity * diffuse;
        vec3 diffContrib = diffuse;
        // With normalization factor
        //vec3 specColor = diffuse ;
        vec3 specColor = vec3(0.03) + vec3(0.97)*pow(1 - dot(normalize(tmpHalfway), lightDir), 5);        
        //vec3 specContrib = normalFactor * pow(max(dot(reflectDir, eyeDir), 0), specExponent) * specColor * specIntensity;
        //vec3 specContrib = normalFactor * pow(max(dot(halfwayDir, normal), 0), specExponent) * specColor * specIntensity;
        vec3 specContrib = normalFactor * pow(max(dot(halfwayDir, normal), 0), max(specExponent, 1)) * specColor;
        vec3 lighting = ambContrib + (diffContrib + specContrib) * lightColor * attenuation * max(intensity, 0);
        outColor = vec4(lighting, 1.0);
        if(dot(lighting, vec3(0.2126, 0.7152, 0.0722)) > 1.0)
        {
           brightColor = vec4(lighting, 1.0);
        }
    }
);


const char* shLightPassFragSrc = GLSL(
    uniform sampler2D gDiffuse;
    uniform sampler2D gNormalSpec;
    uniform sampler2D gPositionDepth;
    uniform sampler2D shadowMap;
    uniform sampler2D ssao;

    in vec2 vTexcoord;
    layout (location = 0) out vec4 outColor;
    layout (location = 1) out vec4 brightColor;        

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
        vec3 posE = texture(gPositionDepth, vTexcoord).rgb;
        vec4 fragPosL = lightSpaceMat * invViewMat * vec4(posE, 1.0);
        vec3 normal = normalize(texture(gNormalSpec, vTexcoord).rgb);
        vec3 diffuse = texture(gDiffuse, vTexcoord).rgb;
        float specExponent = texture(gDiffuse, vTexcoord).a * 255.0;        
        float occlusion = texture(ssao, vTexcoord).r;
        vec3 lightColor = vec3(500.0);        
        
        vec3 ambContrib = diffuse * 0.3 * occlusion;
        vec3 lightDist = light1 - posE;
        float attenuation = 1/dot(lightDist, lightDist);
        vec3 lightDir = normalize(lightDist);        
        vec3 eyeDir = normalize(-posE);
        float intensity = dot(normal, lightDir);
        vec3 tmpHalfway = lightDir + eyeDir;
        vec3 halfwayDir = normalize(tmpHalfway * clamp(intensity, 0, 1));        

        float normalFactor = (specExponent + 8) * 0.125;        
        vec3 diffContrib = diffuse;
        vec3 specColor = vec3(0.03) + vec3(0.97)*pow(1 - dot(normalize(tmpHalfway), lightDir), 5);                
        vec3 specContrib = normalFactor * pow(max(dot(halfwayDir, normal), 0), max(specExponent, 1.0)) * specColor;        
        float shadow = ShadowCalculation(fragPosL);
        vec3 lighting = ambContrib + (1.0 - shadow) * ((diffContrib + specContrib) * lightColor * attenuation * max(intensity, 0));
        outColor = vec4(lighting, 1.0);
        if(dot(lighting, vec3(0.2126, 0.7152, 0.0722)) > 1.0)
        {
           brightColor = vec4(lighting, 1.0);
        }        
    }
);

const char* SSAOFragSrc = GLSL(

    in vec2 vTexcoord;

    uniform sampler2D gPositionDepth;
    uniform sampler2D gNormalSpec;
    uniform sampler2D texNoise;

    uniform vec3 samples[64];
    uniform int kernelSize;
    uniform float radius;

    const vec2 noiseScale = vec2(1280.0f / 4.0f, 720.0f / 4.0f);
    
    out float outColor;
    void main()
    {
        vec3 posE = texture(gPositionDepth, vTexcoord).xyz;
        vec3 normal = texture(gNormalSpec, vTexcoord).xyz;
        vec3 randomVec = texture(texNoise, vTexcoord * noiseScale).xyz;

        vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
        vec3 biTangent = cross(normal, tangent);
        mat3 TBN = mat3(tangent, biTangent, normal);

        float occlusion = 0.0;
        for(int i = 0; i < kernelSize; i++)
        {
            vec3 sample = TBN * samples[i];
            sample = posE + sample * radius;

            vec4 offset = vec4(sample, 1.0);
            offset = projMat * offset;
            offset.xyz /= offset.w;
            offset.xyz = offset.xyz * 0.5 + 0.5;

            float sampleDepth = -texture(gPositionDepth, offset.xy).w;

            float rangeCheck = smoothstep(0.0, 1.0, radius / abs(posE.z - sampleDepth));
            occlusion += (sampleDepth >= sample.z ? 1.0 : 0.0) * rangeCheck;
        }
        occlusion = 1.0 - (occlusion / kernelSize);
        outColor = occlusion;
    }
);

const char* SSAOBlurFragSrc = GLSL(
    in vec2 vTexcoord;

    uniform sampler2D ssao;

    out float outColor;

    void main()
    {
        vec2 texelSize = 1.0 / vec2(textureSize(ssao, 0));
        float result = 0.0;
        for(int x = -2; x < 2; ++x)
        {
            for(int y = -2; y < 2; ++y)
            {
                vec2 offset = vec2(float(x), float(y)) * texelSize;
                result += texture(ssao, vTexcoord + offset).r;
            }
        }
        outColor = result / 16.0;
    }
);

const char* BloomBlurFragSrc = GLSL(
    in vec2 vTexcoord;

    uniform sampler2D brightColorBuffer;

    uniform bool horizontal;

    uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

    out vec4 outColor;
    
    void main()
    {
        vec2 tex_offset = 1.0 / textureSize(brightColorBuffer, 0);
        vec3 result = texture(brightColorBuffer, vTexcoord).rgb * weight[0];
        if(horizontal)
        {
            for(int i = 1; i < 5; i++)
            {
                result += texture(brightColorBuffer, vTexcoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
                result += texture(brightColorBuffer, vTexcoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];                
            }
        }else
        {
            for(int i = 1; i < 5; i++)
            {
                result += texture(brightColorBuffer, vTexcoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
                result += texture(brightColorBuffer, vTexcoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];                
            }            
        }
        outColor = vec4(result, 1.0);
    }
);

const char* showSSAOFragSrc = GLSL(
    uniform sampler2D ssao;    

    in vec2 vTexcoord;
    out vec4 outColor;

    void main()
    {
        float occlusion = texture(ssao, vTexcoord).r;        
        outColor = vec4(occlusion, occlusion, occlusion, 1.0);
    }
);

const char* showNormalFragSrc = GLSL(
    uniform sampler2D gNormalSpec;
    
    in vec2 vTexcoord;
    out vec4 outColor;

    void main()
    {
        vec3 normal = normalize(texture(gNormalSpec, vTexcoord).rgb);
        outColor = vec4(normal, 1.0);
    }
);

const char* showDiffuseFragSrc = GLSL(
    uniform sampler2D gDiffuse;

    in vec2 vTexcoord;
    out vec4 outColor;

    void main()
    {
        vec3 diffuse = texture(gDiffuse, vTexcoord).rgb;
        outColor = vec4(diffuse, 1.0);
    }
);

const char* showSpecularFragSrc = GLSL(
    uniform sampler2D gDiffuse;
    
    in vec2 vTexcoord;
    out vec4 outColor;

    void main()
    {
        float specular = texture(gDiffuse, vTexcoord).a;
        outColor = vec4(specular, specular, specular, 1.0);
    }
);

const char* exposureHDRFragSrc = GLSL(
    in vec2 vTexcoord;

    uniform sampler2D hdrBuffer;
    uniform float exposure;
    out vec4 outColor;
    
    void main()
    {
        const float gamma = 1.0;
        vec3 hdrColor = texture(hdrBuffer, vTexcoord).rgb;
        vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
        mapped = pow(mapped, vec3(1.0 / gamma));
         outColor = vec4(mapped, 1.0);
    }

);

const char* bloomFragSrc = GLSL(
    in vec2 vTexcoord;

    uniform sampler2D hdrBuffer;
    uniform sampler2D bloomBlurBuffer;
    uniform float exposure;
    out vec4 outColor;
    
    void main()
    {
        const float gamma = 1.0;
        vec3 hdrColor = texture(hdrBuffer, vTexcoord).rgb;
        vec3 bloomColor = texture(bloomBlurBuffer, vTexcoord).rgb;
        hdrColor += bloomColor;
        vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
        mapped = pow(mapped, vec3(1.0 / gamma));
        outColor = vec4(mapped, 1.0);
    }
);

const char* showBloomFragSrc = GLSL(
    in vec2 vTexcoord;

    uniform sampler2D bloomBlurBuffer;
    uniform float exposure;
    out vec4 outColor;
    
    void main()
    {
        const float gamma = 1.0;
        vec3 bloomColor = texture(bloomBlurBuffer, vTexcoord).rgb;
        vec3 mapped = vec3(1.0) - exp(-bloomColor * exposure);
        mapped = pow(mapped, vec3(1.0 / gamma));
        outColor = vec4(mapped, 1.0);
    }
);

#endif
