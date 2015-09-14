#ifndef SSHADERS_H
#define SSHADERS_H

#include <GL/glew.h>

#define GLSL(src) "#version 330 core\n\
layout (std140) uniform UniformBlock{\
mat4 projMat;\
vec3 light1;\
mat4 lightSpaceMat;\
mat4 invViewMat;\
};\n" #src

const char* shadowVertSrc = GLSL(
    uniform mat4 uModelMat;
    uniform mat4 uViewMat;

    layout (location = 0) in vec3 aPosition;
    layout (location = 1) in vec3 aNormal;
    layout (location = 2) in vec2 aTexcoord;

    // vPosition and vNormal are in world space.
    out vec3 vPosition;
    out vec3 vNormal;
    out vec2 vTexcoord;
    out vec4 vFragPosLightSpace;

    out vec3 vLightW;
    out vec3 vEyeW;

    void main()
    {        
        vPosition = (uModelMat * vec4(aPosition, 1.0)).xyz;
        vNormal = (transpose(inverse(uModelMat)) * vec4(aNormal, 1.0)).xyz;
        vTexcoord = aTexcoord;
        vFragPosLightSpace = lightSpaceMat * vec4(vPosition, 1.0);
        mat4 invViewMat = inverse(uViewMat);
        vLightW = (invViewMat * vec4(light1, 1.0)).xyz;
        vEyeW = (invViewMat * vec4(0, 0, 0, 1)).xyz;
        gl_Position = projMat * uViewMat * vec4(vPosition, 1.0);
    }
);

const char* shOBJFragSrc = GLSL(
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;
    in vec4 vFragPosLightSpace;
    in vec3 vLightW;
    in vec3 vEyeW;

    uniform sampler2D diffuseMap;        // diffuse map
    uniform sampler2D shadowMap;        // shadow map
    uniform vec3 Ka;
    uniform vec3 Kd;
    uniform vec3 Ks;
    uniform float Ns;
    
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
        vec2 texelSize = 1.0 / textureSize(diffuseMap, 0);
        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
            }    
        }
        shadow /= 9.0;        
        //float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;
        return shadow;
    }
    
    void main()
    {
        vec3 texColor = texture(diffuseMap, vTexcoord).rgb;
        vec3 normal = normalize(vNormal);
        
        // Ambient
        vec3 ambContrib = 0.3 * texColor * Kd;
        
        // Diffuse
        vec3 lightDir = normalize(vLightW - vPosition);
        vec3 diffContrib = max(dot(lightDir, normal), 0.0) * texColor * Kd;

        // Specular
        vec3 eyeDir = normalize(vEyeW - vPosition);
        //vec3 reflectDir = reflect(-lightDir, normal);
        vec3 reflectDir = 2*dot(lightDir, normal)*normal - lightDir;

        //vec3 halfwayDir = normalize(lightDir + viewDir);  
        //spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
        vec3 specContrib = pow(max(dot(eyeDir, reflectDir), 0), Ns) * texColor * Kd;    
        // Calculate shadow
        float shadow = ShadowCalculation(vFragPosLightSpace);       
        vec3 lighting = ambContrib + (1.0 - shadow) * (diffContrib + specContrib);    
    
        outColor = vec4(lighting, 1.0f);
        //outColor = vec4(vec3(shadow), 1.0f);        
    }
);

// Vertex shader that calculates the orthonormal basis for normal mapping
const char* shadowNormalVertSrc = GLSL(    
    uniform mat4 uModelMat;
    uniform mat4 uViewMat;

    layout (location = 0) in vec3 aPosition;
    layout (location = 1) in vec3 aNormal;
    layout (location = 2) in vec2 aTexcoord;    

    layout (location = 3) in vec3 aTangent;
    layout (location = 4) in vec3 aBinormal;
    layout (location = 5) in float aDet;    


    out vec3 vLightT;
    out vec3 vEyeT;
    out vec2 vTexcoord;
    out vec4 vFragPosLightSpace;

    out vec3 vLightW;
    out vec3 vEyeW;
    
    void main()
    {
        vTexcoord = aTexcoord;
        mat4 inverseMat = inverse(uViewMat * uModelMat);
        vec3 lightM = (inverseMat * vec4(light1, 1.0)).xyz - aPosition;        
        vec3 eyeM = (inverseMat * vec4(0.0, 0.0, 0.0, 1.0)).xyz - aPosition;

        vEyeT.x = dot(eyeM, aTangent);
        vEyeT.y = dot(eyeM, aBinormal * aDet);
        vEyeT.z = dot(eyeM, aNormal);

        vLightT.x = dot(lightM, aTangent);
        vLightT.y = dot(lightM, aBinormal * aDet);
        vLightT.z = dot(lightM, aNormal);

        vec4 posW = uModelMat * vec4(aPosition, 1.0);
        vFragPosLightSpace = lightSpaceMat * posW;

        gl_Position = projMat * uViewMat * posW;
    }
);

// OBJ material and Phong lighting.
const char* shOBJSpecFragSrc = GLSL(
    uniform sampler2D diffuseMap;
    uniform sampler2D specularMap;
    uniform sampler2D shadowMap;

    uniform vec3 Ka;
    uniform vec3 Kd;
    uniform vec3 Ks;
    uniform float Ns;
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;    
    in vec4 vFragPosLightSpace;
    
    in vec3 vLightW;
    in vec3 vEyeW;    

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
        vec2 texelSize = 1.0 / textureSize(diffuseMap, 0);
        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
            }    
        }
        shadow /= 9.0;        
        //float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;
        return shadow;
    }    

    void main()
    {
        vec3 texColor = texture(diffuseMap, vTexcoord).rgb;
        vec3 normal = normalize(vNormal);
        vec3 specular = texture(specularMap, vTexcoord).rgb;
        
        // Ambient
        vec3 ambContrib = 0.3 * texColor * Kd;
        
        // Diffuse
        vec3 lightDir = normalize(vLightW - vPosition);
        vec3 diffContrib = max(dot(lightDir, normal), 0.0) * texColor * Kd;

        // Specular
        vec3 eyeDir = normalize(vEyeW - vPosition);
        vec3 reflectDir = 2*dot(lightDir, normal)*normal - lightDir;

        vec3 specContrib = pow(max(dot(eyeDir, reflectDir), 0), Ns) * specular * Kd;    
        // Calculate shadow
        float shadow = ShadowCalculation(vFragPosLightSpace);       
        vec3 lighting = ambContrib + (1.0 - shadow) * (diffContrib + specContrib);
        
        outColor = vec4(lighting, 1.0f);
    }
);

// OBJ material and Phong lighting.
const char* shOBJAlphaFragSrc = GLSL(
    uniform sampler2D diffuseMap;
    uniform sampler2D alphaMap;
    uniform sampler2D shadowMap;

    uniform vec3 Ka;
    uniform vec3 Kd;
    //uniform vec3 Ks;
    uniform float Ns;
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;
    in vec4 vFragPosLightSpace;
    
    in vec3 vLightW;
    in vec3 vEyeW;

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
        vec2 texelSize = 1.0 / textureSize(diffuseMap, 0);
        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
            }    
        }
        shadow /= 9.0;        
        //float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;
        return shadow;
    }    

    void main()
    {
        vec3 texColor = texture(diffuseMap, vTexcoord).rgb;
        vec3 normal = normalize(vNormal);
        vec4 alpha = texture(alphaMap, vTexcoord);
        
        // Ambient
        vec3 ambContrib = 0.3 * texColor * Kd;
        
        // Diffuse
        vec3 lightDir = normalize(vLightW - vPosition);
        vec3 diffContrib = max(dot(lightDir, normal), 0.0) * texColor * Kd;

        // Specular
        vec3 eyeDir = normalize(vEyeW - vPosition);
        vec3 reflectDir = 2*dot(lightDir, normal)*normal - lightDir;

        vec3 specContrib = pow(max(dot(eyeDir, reflectDir), 0), Ns) * texColor * Kd;    
        // Calculate shadow
        float shadow = ShadowCalculation(vFragPosLightSpace);       
        vec3 lighting = ambContrib + (1.0 - shadow) * (diffContrib + specContrib);
        if(alpha.r < 0.1)
            discard;
    
        outColor = vec4(lighting, 1.0f);
    }
);

const char* shOBJAlphaSpecFragSrc = GLSL(
    uniform sampler2D diffuseMap;
    uniform sampler2D specularMap;
    uniform sampler2D alphaMap;
    uniform sampler2D shadowMap;    

    uniform vec3 Ka;
    uniform vec3 Kd;
    uniform vec3 Ks;
    uniform float Ns;
    
    in vec3 vPosition;
    in vec3 vNormal;
    in vec2 vTexcoord;    
    in vec4 vFragPosLightSpace;
    
    in vec3 vLightW;
    in vec3 vEyeW;    

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
        vec2 texelSize = 1.0 / textureSize(diffuseMap, 0);
        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
            }    
        }
        shadow /= 9.0;        
        //float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;
        return shadow;
    }    

    void main()
    {
        vec3 texColor = texture(diffuseMap, vTexcoord).rgb;
        vec3 normal = normalize(vNormal);
        vec3 specular = texture(specularMap, vTexcoord).rgb;
        vec4 alpha = texture(alphaMap, vTexcoord);
        
        // Ambient
        vec3 ambContrib = 0.3 * texColor * Kd;
        
        // Diffuse
        vec3 lightDir = normalize(vLightW - vPosition);
        vec3 diffContrib = max(dot(lightDir, normal), 0.0) * texColor * Kd;

        // Specular
        vec3 eyeDir = normalize(vEyeW - vPosition);
        vec3 reflectDir = 2*dot(lightDir, normal)*normal - lightDir;

        vec3 specContrib = pow(max(dot(eyeDir, reflectDir), 0), Ns) * specular * Kd;    
        // Calculate shadow
        float shadow = ShadowCalculation(vFragPosLightSpace);       
        vec3 lighting = ambContrib + (1.0 - shadow) * (diffContrib + specContrib);
        if(alpha.r < 0.1)
            discard;
        
        outColor = vec4(lighting, 1.0f);
    }
);

// Normal mapping, OBJ material, and Phong lighting.
const char* shOBJNormalFragSrc = GLSL(

    uniform vec3 Ka;
    uniform vec3 Kd;
    uniform vec3 Ks;
    uniform float Ns;
    uniform sampler2D diffuseMap;
    uniform sampler2D normalMap;
    uniform sampler2D shadowMap;

    in vec3 vLightT;
    in vec3 vEyeT;
    in vec2 vTexcoord;
    in vec4 vFragPosLightSpace;

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
        vec2 texelSize = 1.0 / textureSize(diffuseMap, 0);
        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
            }    
        }
        shadow /= 9.0;        
        //float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;
        return shadow;
    }    
    
    void main()
    {
        vec4 texColor = texture(diffuseMap, vTexcoord);
        
        vec3 lightT = normalize(vLightT);
        
        vec3 ambContrib = 0.3 * Kd * texColor.xyz;       

        vec3 normal = normalize(texture(normalMap, vTexcoord).xyz);

        float intensity = dot(normal, lightT);
        vec3 diffContrib = max(intensity, 0) * texColor.xyz ;
        
        vec3 eyeT = normalize(vEyeT);
        vec3 reflectDir = 2*intensity*normal - lightT;
        vec3 specContrib = pow(max(dot(reflectDir, eyeT), 0.0), Ns) * texColor.xyz * Kd * 0.005;
        float shadow = ShadowCalculation(vFragPosLightSpace);
        vec3 lighting = ambContrib + (1.0 - shadow) * (diffContrib + specContrib);        
        outColor = vec4(lighting, 1.0);
        //outColor = vec4(ambContrib + diffContrib + specContrib, 1.0);
    }
);

const char* shOBJNormalSpecFragSrc = GLSL(

    uniform vec3 Ka;
    uniform vec3 Kd;
    uniform vec3 Ks;
    uniform float Ns;
    uniform sampler2D diffuseMap;
    uniform sampler2D normalMap;
    uniform sampler2D specularMap;
    uniform sampler2D shadowMap;

    in vec3 vLightT;
    in vec3 vEyeT;
    in vec2 vTexcoord;
    in vec4 vFragPosLightSpace;

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
        vec2 texelSize = 1.0 / textureSize(diffuseMap, 0);
        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
            }    
        }
        shadow /= 9.0;        
        //float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;
        return shadow;
    }    
    
    void main()
    {
        vec4 texColor = texture(diffuseMap, vTexcoord);
        
        vec3 lightT = normalize(vLightT);
        
        vec3 ambContrib = 0.3 * Kd * texColor.xyz;       

        vec3 normal = normalize(texture(normalMap, vTexcoord).xyz);

        float intensity = dot(normal, lightT);
        vec3 diffContrib = max(intensity, 0) * texColor.xyz ;
        
        vec3 eyeT = normalize(vEyeT);
        vec3 reflectDir = 2*intensity*normal - lightT;
        vec4 specular = texture(specularMap, vTexcoord);
        vec3 specContrib = pow(max(dot(reflectDir, eyeT), 0.0), Ns) * specular.rgb * Kd * 0.005;
        float shadow = ShadowCalculation(vFragPosLightSpace);
        vec3 lighting = ambContrib + (1.0 - shadow) * (diffContrib + specContrib);        
        outColor = vec4(lighting, 1.0);
        //outColor = vec4(ambContrib + diffContrib + specContrib, 1.0);
    }
);

const char* shOBJNormalAlphaFragSrc = GLSL(

    uniform vec3 Ka;
    uniform vec3 Kd;
    uniform vec3 Ks;
    uniform float Ns;
    uniform sampler2D diffuseMap;
    uniform sampler2D normalMap;
    uniform sampler2D alphaMap;
    uniform sampler2D shadowMap;

    in vec3 vLightT;
    in vec3 vEyeT;
    in vec2 vTexcoord;
    in vec4 vFragPosLightSpace;

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
        vec2 texelSize = 1.0 / textureSize(diffuseMap, 0);
        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
            }    
        }
        shadow /= 9.0;        
        //float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;
        return shadow;
    }    
    
    void main()
    {
        vec4 texColor = texture(diffuseMap, vTexcoord);
        
        vec3 lightT = normalize(vLightT);
        
        vec3 ambContrib = 0.3 * Kd * texColor.xyz;       

        vec3 normal = normalize(texture(normalMap, vTexcoord).xyz);

        float intensity = dot(normal, lightT);
        vec3 diffContrib = max(intensity, 0) * texColor.xyz ;
        
        vec3 eyeT = normalize(vEyeT);
        vec3 reflectDir = 2*intensity*normal - lightT;
        vec3 specContrib = pow(max(dot(reflectDir, eyeT), 0.0), Ns) * texColor.xyz * Kd * 0.005;
        float shadow = ShadowCalculation(vFragPosLightSpace);
        vec3 lighting = ambContrib + (1.0 - shadow) * (diffContrib + specContrib);
        vec4 alpha = texture(alphaMap, vTexcoord);
        if(alpha.r < 0.1)
            discard;
        outColor = vec4(lighting, 1.0);
        //outColor = vec4(ambContrib + diffContrib + specContrib, 1.0);
    }
);

const char* shOBJNormalAlphaSpecFragSrc = GLSL(

    uniform vec3 Ka;
    uniform vec3 Kd;
    uniform vec3 Ks;
    uniform float Ns;
    uniform sampler2D diffuseMap;
    uniform sampler2D normalMap;
    uniform sampler2D specularMap;
    uniform sampler2D alphaMap;
    uniform sampler2D shadowMap;

    in vec3 vLightT;
    in vec3 vEyeT;
    in vec2 vTexcoord;
    in vec4 vFragPosLightSpace;

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
        vec2 texelSize = 1.0 / textureSize(diffuseMap, 0);
        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
            }    
        }
        shadow /= 9.0;        
        //float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;
        return shadow;
    }    
    
    void main()
    {
        vec4 texColor = texture(diffuseMap, vTexcoord);
        
        vec3 lightT = normalize(vLightT);
        
        vec3 ambContrib = 0.3 * Kd * texColor.xyz;       

        vec3 normal = normalize(texture(normalMap, vTexcoord).xyz);

        float intensity = dot(normal, lightT);
        vec3 diffContrib = max(intensity, 0) * texColor.xyz ;
        
        vec3 eyeT = normalize(vEyeT);
        vec3 reflectDir = 2*intensity*normal - lightT;
        vec4 specular = texture(specularMap, vTexcoord);
        vec3 specContrib = pow(max(dot(reflectDir, eyeT), 0.0), Ns) * specular.rgb * Kd * 0.005;
        float shadow = ShadowCalculation(vFragPosLightSpace);
        vec3 lighting = ambContrib + (1.0 - shadow) * (diffContrib + specContrib);
        vec4 alpha = texture(alphaMap, vTexcoord);
        if(alpha.r < 0.1)
            discard;        
        outColor = vec4(lighting, 1.0);
        //outColor = vec4(ambContrib + diffContrib + specContrib, 1.0);
    }
);

#endif
