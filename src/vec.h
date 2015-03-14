#include <math.h>
// TODO: add assert equivalents
typedef union{
    float v[2];
    struct{
        float x;
        float y;        
    };
}vec2;

typedef union{
    float v[3];
    struct{
        float x;
        float y;
        float z;        
    };
}vec3;

typedef union{
    float v[4];
    struct{
        float x;
        float y;
        float z;
        float a;        
    };
}vec4;

// Addition and subtraction

void vec2Add(vec2 *output, vec2 *a, vec2 *b)
{
    output->x = a->x + b->x;
    output->y = a->y + b->y;
}

void vec2Sub(vec2 *output, vec2 *a, vec2 *b)
{
    vec2 tmp;
    tmp.x = -b->x;
    tmp.y = -b->y;
    vec2Add(output, a, &tmp);
}

void vec3Add(vec3 *output, vec3 *a, vec3 *b)
{
    output->x = a->x + b->x;
    output->y = a->y + b->y;
    output->z = a->z + b->z;
}

void vec3Sub(vec3 *output, vec3 *a, vec3 *b)
{
    vec3 tmp;
    tmp.x = -b->x;
    tmp.y = -b->y;
    tmp.z = -b->z;
    vec3Add(output, a, &tmp);
}

// TODO: decide what to do with 4 element vectors
void vec4Add(vec4 *output, vec4 *a, vec4 *b)
{
    output->x = a->x + b->x;
    output->y = a->y + b->y;
    output->z = a->z + b->z;
    output->a = a->a + b->a;
}

void vec4Sub(vec4 *output, vec4 *a, vec4 *b)
{
    vec4 tmp;
    tmp.x = -b->x;
    tmp.y = -b->y;
    tmp.z = -b->z;
    tmp.a = -b->a;
    vec4Add(output, a, &tmp);
}

void vec2Mult(vec2 *output, vec2 *a, float b)
{
    output->x = a->x*b;
    output->y = a->y*b;
}

void vec3Mult(vec3 *output, vec3 *a, float b)
{
    output->x = a->x*b;
    output->y = a->y*b;
    output->z = a->z*b;
}

void vec4Mult(vec4 *output, vec4 *a, float b)
{
    output->x = a->x*b;
    output->y = a->y*b;
    output->z = a->z*b;
    output->a = a->a*b;
}

void vec2Div(vec2 *output, vec2 *a, float b)
{
    vec2Mult(output, a, 1/b);
}

void vec3Div(vec3 *output, vec3 *a, float b)
{
    vec3Mult(output, a, 1/b);
}

void vec4Div(vec4 *output, vec4 *a, float b)
{
    vec4Mult(output, a, 1/b);
}

float dotVec2(vec2 *a, vec2 *b)
{
    return (a->x*b->x + a->y*b->y);
}

float dotVec3(vec3 *a, vec3 *b)
{
    return (a->x*b->x + a->y*b->y + a->z*b->z);
}

float dotVec4(vec4 *a, vec4 *b)
{
    return (a->x*b->x + a->y*b->y + a->z*b->z + a->a*b->a);
}

void normalizeVec2(vec2 *output, vec2 *input)
{
    float tmp;
    tmp = dotVec2(input, input);
    vec2Div(output, input, sqrt(tmp));
}

void normalizeVec3(vec3 *output, vec3 *input)
{
    float tmp;
    tmp = dotVec3(input, input);
    vec3Div(output, input, sqrt(tmp));
}

void normalizeVec4(vec4 *output, vec4 *input)
{
    float tmp;
    tmp = dotVec4(input, input);
    vec4Div(output, input, sqrt(tmp));
}

void cross(vec3 *output, vec3 *a, vec3 *b)
{
    output->x = a->y*b->z - a->z*b->y;
    output->y = a->z*b->x - a->x*b->z;
    output->z = a->x*b->y - a->y*b->x;
}

// TODO: test norm functions
float normVec2(vec2 *a)
{
    return sqrt(dotVec2(a, a));
}

float normVec3(vec3 *a)
{
    return sqrt(dotVec3(a, a));
}

float normVec4(vec4 *a)
{
    return sqrt(dotVec4(a, a));
}
