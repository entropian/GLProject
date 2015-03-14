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


