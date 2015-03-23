#ifndef QUAT_H
#define QUAT_H

#include <cassert>
#include <cmath>
#include <iostream>

//#include "vec.h"
#include "mat.h"

// TODO: quat difference

class Quat;
Quat inv(const Quat& q);

class Quat
{
    Vec4 q_;

public:
    float operator [](const int i) const
    {
        return q_[i];
    }

    float& operator [] (const int i)
    {
        return q_[i];
    }

    Quat()
        :q_(1, 0, 0, 0){}

    Quat(const float w, const Vec3& v)
        : q_(w, v[0], v[1], v[2]){}

    Quat(const float w, const float x, const float y, const float z)
        : q_(w, x, y, z){}

    Quat& operator += (const Quat& a)
    {
        q_ += a.q_;
        return *this;
    }

    Quat& operator -= (const Quat& a)
    {
        q_ -= a.q_;
        return *this;
    }

    Quat& operator *= (const float a)
    {
        q_ *= a;
        return *this;
    }

    Quat& operator /= (const float a)
    {
        q_ /= a;
        return *this;
    }

    Quat operator + (const Quat& a) const
    {
        return Quat(*this) += a;        
    }

    Quat operator - (const Quat& a) const
    {
        return Quat(*this) -= a;        
    }

    Quat operator * (const float a) const
    {
        return Quat(*this) *= a;
    }

    Quat operator / (const float a) const
    {
        return Quat(*this) /= a;
    }

    Quat operator * (const Quat& a) const
    {
        const Vec3 u(q_[1], q_[2], q_[3]), v(a.q_[1], a.q_[2], a.q_[3]);
        return Quat(q_[0]*a.q_[0] - dot(u, v), (v*q_[0] + u*a.q_[0]) + cross(u, v));
    }

    Vec3 operator * (const Vec3& a) const
    {
        const Quat r = *this * (Quat(0, a[0], a[1], a[2]) * inv(*this));
        return Vec3(r[1], r[2], r[3]);
    }

    static Quat makeXRotation(const float ang)
    {
        Quat r;
        const float halfAngle = (float)(ang*0.5*PI/180);
        r.q_[0] = (float)cos(halfAngle);
        r.q_[1] = (float)sin(halfAngle);
        return r;
    }

    static Quat makeYRotation(const float ang)
    {
        Quat r;
        const float halfAngle = (float)(ang*0.5*PI/180);
        r.q_[0] = (float)cos(halfAngle);
        r.q_[2] = (float)sin(halfAngle);
        return r;
    }

    static Quat makeZRotation(const float ang)
    {
        Quat r;
        const float halfAngle = (float)(ang*0.5*PI/180);
        r.q_[0] = (float)cos(halfAngle);
        r.q_[3] = (float)sin(halfAngle);
        return r;
    }
};

inline float dot(const Quat& q, const Quat& p)
{
    return (q[0]*p[0] + q[1]*p[1] + q[2]*p[2] + q[3]*p[3]);
}

inline float norm2(const Quat& q)
{
    return dot(q, q);
}

inline Quat inv(const Quat& q)
{
    const float n = sqrt(norm2(q));
    assert(n > EPS2);
    return Quat(q[0], -q[1], -q[2], -q[3])*(float)(1.0/n);
}

inline Quat normalize(const Quat& q)
{
    const float n = sqrt(norm2(q));
    return q/n;
}

inline Mat4 quatToMat(const Quat& q)
{
    Mat4 r;
    // TODO: make sure that the sqrt works for normalizing
    const float n = sqrt(norm2(q));
    if(n < EPS2)
        return Mat4(0);

    const float two_over_n = 2/n;
    r(0, 0) -= (q[2]*q[2] + q[3]*q[3])*two_over_n;
    r(0, 1) += (q[1]*q[2] - q[0]*q[3])*two_over_n;
    r(0, 2) += (q[1]*q[3] + q[0]*q[2])*two_over_n;
    r(1, 0) += (q[1]*q[2] + q[0]*q[3])*two_over_n;
    r(1, 1) -= (q[1]*q[1] + q[3]*q[3])*two_over_n;
    r(1, 2) += (q[2]*q[3] - q[0]*q[1])*two_over_n;
    r(2, 0) += (q[1]*q[3] - q[0]*q[2])*two_over_n;
    r(2, 1) += (q[2]*q[3] + q[0]*q[1])*two_over_n;
    r(2, 2) -= (q[1]*q[1] + q[2]*q[2])*two_over_n;

    assert(isAffine(r));
    return r;
}

inline Quat slerp(const Quat &src, const Quat& dest, float t)
{
    Quat r;
    return r;
}

inline Quat power(const Quat& a, float exponent)
{
    Quat r;

    return r;
}

#endif
