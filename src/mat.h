#include "vec.h"

// TODO: test this
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
void transpose(mat4 *output, mat4 *input)
{
    output->ax = input->ax;
    output->ay = input->bx;
    output->az = input->cx;
    output->aa = input->dx;
    output->bx = input->ay;
    output->by = input->by;
    output->bz = input->cy;
    output->ba = input->dy;
    output->cx = input->az;
    output->cy = input->bz;
    output->cz = input->cz;
    output->ca = input->dz;
    output->dx = input->aa;
    output->dy = input->ba;
    output->dz = input->ca;
    output->da = input->da;
}

// TODO: test this
void multScalar(mat4 *output, mat4 *a, float b)
{
    int i;
    for(i = 0; i < 16; i++)
    {
        output->d_[i] = a->d_[i] * b;
    }
}
