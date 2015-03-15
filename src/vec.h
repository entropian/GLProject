#ifndef VEC_H
#define VEC_H

#include <math.h>
#include <assert.h>

// TODO: add assert equivalents
typedef union{
    float v_[2];
    struct{
        float x;
        float y;        
    };
}vec2;

typedef union{
    float v_[3];
    struct{
        float x;
        float y;
        float z;        
    };
}vec3;

typedef union{
    float v_[4];
    struct{
        float x;
        float y;
        float z;
        float a;        
    };
}vec4;

// Addition and subtraction

void V2Add(vec2 *output, vec2 *a, vec2 *b)
{
    output->x = a->x + b->x;
    output->y = a->y + b->y;
}

void V2Sub(vec2 *output, vec2 *a, vec2 *b)
{
    vec2 tmp;
    tmp.x = -b->x;
    tmp.y = -b->y;
    V2Add(output, a, &tmp);
}

void V3Add(vec3 *output, vec3 *a, vec3 *b)
{
    output->x = a->x + b->x;
    output->y = a->y + b->y;
    output->z = a->z + b->z;
}

void V3Sub(vec3 *output, vec3 *a, vec3 *b)
{
    vec3 tmp;
    tmp.x = -b->x;
    tmp.y = -b->y;
    tmp.z = -b->z;
    V3Add(output, a, &tmp);
}

// TODO: decide what to do with 4 element vectors
void V4Add(vec4 *output, vec4 *a, vec4 *b)
{
    output->x = a->x + b->x;
    output->y = a->y + b->y;
    output->z = a->z + b->z;
    output->a = a->a + b->a;
}

void V4Sub(vec4 *output, vec4 *a, vec4 *b)
{
    vec4 tmp;
    tmp.x = -b->x;
    tmp.y = -b->y;
    tmp.z = -b->z;
    tmp.a = -b->a;
    V4Add(output, a, &tmp);
}

void V2Mult(vec2 *output, vec2 *a, float b)
{
    output->x = a->x*b;
    output->y = a->y*b;
}

void V3Mult(vec3 *output, vec3 *a, float b)
{
    output->x = a->x*b;
    output->y = a->y*b;
    output->z = a->z*b;
}

void V4Mult(vec4 *output, vec4 *a, float b)
{
    output->x = a->x*b;
    output->y = a->y*b;
    output->z = a->z*b;
    output->a = a->a*b;
}

void V2Div(vec2 *output, vec2 *a, float b)
{
    V2Mult(output, a, 1/b);
}

void V3Div(vec3 *output, vec3 *a, float b)
{
    V3Mult(output, a, 1/b);
}

void V4Div(vec4 *output, vec4 *a, float b)
{
    V4Mult(output, a, 1/b);
}

float DotV2(vec2 *a, vec2 *b)
{
    return (a->x*b->x + a->y*b->y);
}

float DotV3(vec3 *a, vec3 *b)
{
    return (a->x*b->x + a->y*b->y + a->z*b->z);
}

float DotV4(vec4 *a, vec4 *b)
{
    return (a->x*b->x + a->y*b->y + a->z*b->z + a->a*b->a);
}

void NormalizeV2(vec2 *output, vec2 *input)
{
    float tmp;
    tmp = dotVec2(input, input);
    vec2Div(output, input, (float)sqrt(tmp));
}

void NormalizeV3(vec3 *output, vec3 *input)
{
    float tmp;
    tmp = dotVec3(input, input);
    vec3Div(output, input, (float)sqrt(tmp));
}

void NormalizeV4(vec4 *output, vec4 *input)
{
    float tmp;
    tmp = DotV4(input, input);
    vec4Div(output, input, (float)sqrt(tmp));
}

void Cross(vec3 *output, vec3 *a, vec3 *b)
{
    output->x = a->y*b->z - a->z*b->y;
    output->y = a->z*b->x - a->x*b->z;
    output->z = a->x*b->y - a->y*b->x;
}

// TODO: test norm functions
float NormV2(vec2 *a)
{
    return (float)sqrt(DotV2(a, a));
}

float NormV3(vec3 *a)
{
    return (float)sqrt(DotV3(a, a));
}

float NormV4(vec4 *a)
{
    return (float)sqrt(DotV4(a, a));
}

