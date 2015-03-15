#include "vec.h"

// Many of the matrix parameters are passed by value to allow the input matrix to also be the output location.
static const float PI = 3.14159265;
static const float EPS = 1e-8;
static const float EPS2 = EPS*EPS;
static const float EPS3 = EPS*EPS*EPS;


// Column major
typedef union{
    float d_[16];

    struct{
        float ax, ay, az, aa;
        float bx, by, bz, ba;
        float cx, cy, cz, ca;
        float dx, dy, dz, da;
    };
}mat4;
// TODO: test this
inline void IDMat(mat4 *output)
{
    int i;
    for(i = 0; i < 16; i++)
        output->d_[i] = 0;
    for(i = 0; i < 4; i++)
        output->d_[i*4 + i] = 1;
}
// TODO: test this
inline void CopyMat(mat4 *output, const mat4 *input)
{
    int i;
    for(i = 0; i < 16; i++)
        output->d_[i] = input->d_[i];
}
// Re-test this to see if pass by value makes a difference or if pass by reference was fine
inline void Transpose(mat4 *output, const mat4 input)
{
    output->ax = input.ax;
    output->ay = input.bx;
    output->az = input.cx;
    output->aa = input.dx;
    output->bx = input.ay;
    output->by = input.by;
    output->bz = input.cy;
    output->ba = input.dy;
    output->cx = input.az;
    output->cy = input.bz;
    output->cz = input.cz;
    output->ca = input.dz;
    output->dx = input.aa;
    output->dy = input.ba;
    output->dz = input.ca;
    output->da = input.da;
}

inline void M4MultScalar(mat4 *output, const mat4 a, const float b)
{
    int i;
    for(i = 0; i < 16; i++)
    {
        output->d_[i] = a.d_[i] * b;
    }
}

// TODO: test this
inline void M4Mult(mat4 *output, const mat4 a, const mat4 b)
{
    int i, j;
    for(i = 0; i < 4; i++)
    {
        vec4 leftRow = {.x = a.d_[i], .y = a.d_[4 + i], .z = a.d_[8 + i], .a = a.d_[12 + i]};
        for(j = 0; j < 4; j++)
        {
            vec4 rightCol = {.x = b.d_[j*4], .y = b.d_[j*4 + 1], .z = b.d_[j*4 + 2], .a = b.d_[j*4 + 3]};
            output->d_[j*4 + 1] = dotVec4(&leftRow, &rightCol);
        }
    }
}

// TODO: test this
inline void M4MultVec3(vec3 *output, const mat4 *a, const vec3 b)
{
    int i;
    for(i = 0; i < 3; i++)
    {
        vec3 leftRow = {.x = a->d_[i], .y = a->d_[4 + i], .z = a->d_[8 + i]};
        output->v_[i] = DotV3(&leftRow, b);
    }
}
// TODO: test these two
inline void M4Add(mat4 *output, const mat4 a, const mat4 b)
{
    int i;
    for(i = 0; i < 16; i++)
        output->d_[i] = a.d_[i] + b.d_[i];
}

inline void M4Add(mat4 *output, const mat4 a, const mat4 b)
{
    int i;
    for(i = 0; i < 16; i++)
        output->d_[i] = a.d_[i] - b.d_[i];
}

// TODO: test the rotation functions
inline void MakeXRotation(mat4 *output, const float ang)
{
    float cosAng = (float)cos(ang * PI / 180.0);
    float sinAng = (float)sin(ang * PI / 180.0);
    
    int i;
    for(i = 0; i < 15; i++)
        output->d_[i] = 0.0;
    
    output->d_[0] = 1.0;
    output->d_[15] = 1.0;
    output->d_[5] = cosAng;
    output->d_[6] = -sinAng;
    output->d_[9] = sinAng;
    output->d_[10] = cosAng;    
}
    

inline void MakeYRotation(mat4 *output, const float ang)
{
    float cosAng = (float)cos(ang * PI / 180.0);
    float sinAng = (float)sin(ang * PI / 180.0);
    
    int i;
    for(i = 1; i < 15; i++)
        output->d_[i] = 0.0;
    
    output->d_[5] = 1.0;
    output->d_[15] = 1.0;
    output->d_[0] = cosAng;
    output->d_[2] = sinAng;
    output->d_[8] = -sinAng;
    output->d_[10] = cosAng;    
}

inline void MakeZRotation(mat4 *output, const float ang)
{
    float cosAng = (float)cos(ang * PI / 180.0);
    float sinAng = (float)sin(ang * PI / 180.0);
    
    int i;
    for(i = 1; i < 15; i++)
        output->d_[i] = 0.0;
    
    output->d_[10] = 1.0;
    output->d_[15] = 1.0;
    output->d_[0] = cosAng;
    output->d_[1] = -sinAng;
    output->d_[4] = sinAng;
    output->d_[5] = cosAng;    
}

// TODO: test this
inline void MakeScale(mat4 *output, const float s)
{
    int i;
    for(i = 0; i < 15; i++)
        output->d_[i] = 0.0;
    
    output->d_[15] = 1.0;
    output->d_[0] = s;
    output->d_[5] = s;
    output->d_[10] = s;
}
// TODO: test this
inline int IsAffine(const Mat4 *input)
{
    if((abs(input->da) + abs(input->ca) + abs(input->ba) + abs(input->aa)) < EPS)
        return 1;
    else
        return 0;
}
// TODO: test this
inline float Norm2M4(const Mat4 *input)
{
    float ret;
    int i;
    for(i = 0; i < 16; i++)
        i += input->d_[i]*input->d_[i];
}
// TODO: test this
inline void M4Inv(mat4 *output, const mat4 input)
{
     float det = input.d_[0](input.d_[5]*input.d_[10] - input.d_[9]*input.d_[6]) +
        input.d_[4](input.d_[9]*input.d_[2] - input.d_[1]*input.d_[10]) +
        input.d_[8](input.d_[1]*input.d_[6] - input.d_[5]*input.d_[2]);

     // assert that |det| > 0?
     int i;
     for(i = 0; i < 15; i++)
         output->d_[i] = 0;
     // Rotation partition
     output->ax = (input.by*input.cz - input.cy*input.bz)/det;
     output->ay = (input.bx*input.cz - input.cx*input.bz)/det;
     output->az = (input.bx*input.cy - input.cx*input.by)/det;
     output->bx = (input.ay*input.cz - input.cy*input.az)/det;
     output->by = (input.ax*input.cz - input.cx*input.az)/det;
     output->bz = (input.ax*input.cy - input.cx*input.ay)/det;
     output->cx = (input.ay*input.bz - input.by*input.az)/det;
     output->cy = (input.ax*input.bz - input.bx*input.az)/det;
     output->cz = (input.ax*input.by - input.bx*input.ay)/det;
     // Translation partition
     output->dx = -(input.dx*output->ax + input.dy*output->bx + input.dz*output->cx);
     output->dy = -(input.dx*output->ay + input.dy*output->by + input.dz*output->cy);
     output->dz = -(input.dx*output->az + input.dy*output->bz + input.dz*output->cz);

     output->da = 1.0;
     // assert
     mat4 tmp1, tmp2;
     IDMat(&tmp);
     M4Mult(&tmp2, input, *output);
     M4Sub(&tmp, &tmp, &tmp2);
     assert(IsAffine(output) == 1 && Norm2M4(&tmp) < EPS2);
}
