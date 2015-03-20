#ifndef MAT_H
#define MAT_H

#include <cassert>
#include <cmath>
//#include "vec.h"

class Mat4
{
    float f_[16];

public:
    //tested?
    float& operator () (const int row, const int col)
    {
        return f_[(row << 2) + col];
    }
    //tested
    const float& operator () (const int row, const int col) const
    {
        return f_[(row << 2) + col];
    }
    //tested?
    float& operator [] (const int i)
    {
        return f_[i];
    }
    //tested
    const float& operator [] (const int i) const
    {
        return f_[i];
    }
    //tested
    Mat4()
    {
        for(int i = 0; i < 16; i++)
            f_[i] = 0;

        for(int i = 0; i < 4; i++)
            f_[(i << 2) + i] = 1;
    }
    //tested
    Mat4(const float a)
    {
        for(int i = 0; i < 16; i++)
            f_[i] = a;
    }
    //tested
    Mat4(const Mat4& m)
    {
        for(int i = 0; i < 16; i++)
            f_[i] = m.f_[i];
    }
    //tested
    Mat4& operator +=(const Mat4& m)
    {
        for(int i = 0; i < 16; i++)
            f_[i] += m.f_[i];
        return *this;
    }
    //tested
    Mat4& operator -=(const Mat4& m)
    {
        for(int i = 0; i < 16; i++)
            f_[i] -= m.f_[i];
        return *this;
    }
    //tested
    Mat4& operator *=(const float a)
    {
        for(int i = 0; i < 16; i++)
            f_[i] *= a;
        return *this;
    }
    //tested
    Mat4& operator *=(const Mat4& m)
    {
        return *this = *this * m;
    }
    //tested
    Mat4 operator + (const Mat4& m) const
    {
        return Mat4(*this) += m;
    }
    //tested
    Mat4 operator - (const Mat4& m) const
    {
        return Mat4(*this) -= m;
    }
    //tested
    Mat4 operator * (const float a) const
    {
        return Mat4(*this) *= a;
    }
    //tested?
    Vec4 operator * (const Vec4& v) const
    {
        Vec4 r(0);
        for(int i = 0; i < 4; i++)
        {
            for(int j = 0; j < 4; j++)
                r[i] += (*this)(i, j)*v[j];
        }
        return r;
    }
    //tested?
    Mat4 operator * (const Mat4& m) const
    {
        Mat4 r(0);
        for(int i = 0; i < 4; i++)
        {
            for(int j = 0; j < 4; j++)
            {
                for(int k = 0; k < 4; k++)
                    r(i, k) += (*this)(i, j) * m(j, k);
            }
        }
        return r;
    }
    
    static Mat4 makeXRotation(const float ang)
    {
        Mat4 r;
        float cosAng = (float)cos(ang * PI / 180.0);
        float sinAng = (float)sin(ang * PI / 180.0);
        r(1, 1) = cosAng;
        r(1, 2) = -sinAng;
        r(2, 1) = sinAng;
        r(2, 2) = cosAng;
        return r;
    }
    //tested?
    static Mat4 makeYRotation(const float ang)
    {
        Mat4 r;
        float cosAng = (float)cos(ang * PI / 180.0);
        float sinAng = (float)sin(ang * PI / 180.0);
        r(0, 0) = cosAng;
        r(0, 2) = sinAng;
        r(2, 0) = -sinAng;
        r(2, 2) = cosAng;
        return r;
    }
    //tested?
    static Mat4 makeZRotation(const float ang)
    {
        Mat4 r;
        float cosAng = (float)cos(ang * PI / 180.0);
        float sinAng = (float)sin(ang * PI / 180.0);
        r(0, 0) = cosAng;
        r(0, 1) = -sinAng;
        r(1, 0) = sinAng;
        r(1, 1) = cosAng;
        return r;
    }
    //tested
    static Mat4 makeTranslation(const Vec3& t)
    {
        Mat4 r;
        for(int i = 0; i < 3; i++)
            r(i, 3) = t[i];
        return r;
    }
    //tested?
    static Mat4 makeScale(const Vec3& s)
    {
        Mat4 r;
        for(int i = 0; i < 3; i++)
            r(i, i) = s[i];
        return r;
    }

    static Mat4 makeProjection(const float fovy, const float aspectRatio, const float near, const float far)
    {
        Mat4 r(0);
        float ang = (float)(fovy*0.5*PI/180.0);
        float f = abs(sin(ang)) < EPS ? 0 : 1/tan(ang);
        if(abs(aspectRatio) > EPS)
            r(0, 0) = f/aspectRatio;

        r(1, 1) = f;

        if(abs(far - near) > EPS)
        {
            // TODO: figure out why the matrix4.h version is positive
            //       and figure out how the glm version works
            //       and read the books
            r(2, 2) = -(far + near)/(far - near);
            r(2, 3) = (float)(-2.0*far*near/(far-near));
        }

        r(3, 2) = -1.0;
        return r;
    }

};
//tested?
inline bool isAffine(const Mat4& m)
{
    return abs(m[15]-1) + abs(m[14]) + abs(m[13]) + abs(m[12]) < EPS;
}
//tesed?
inline float norm2(const Mat4& m)
{
    float r = 0;
    for(int i = 0; i < 16; i++)
        r += m[i]*m[i];
    return r;
}
// we'll see
// computes inverse of affine matrix. assumes last row is [0,0,0,1]
inline Mat4 inv(const Mat4& m) {
  Mat4 r;                                              // default constructor initializes it to identity
  assert(isAffine(m));
  float det = m(0,0)*(m(1,1)*m(2,2) - m(1,2)*m(2,1)) +
               m(0,1)*(m(1,2)*m(2,0) - m(1,0)*m(2,2)) +
               m(0,2)*(m(1,0)*m(2,1) - m(1,1)*m(2,0));

  // check non-singular matrix
  assert(abs(det) > EPS3);

  // "rotation part"
  r(0,0) =  (m(1,1) * m(2,2) - m(1,2) * m(2,1)) / det;
  r(1,0) = -(m(1,0) * m(2,2) - m(1,2) * m(2,0)) / det;
  r(2,0) =  (m(1,0) * m(2,1) - m(1,1) * m(2,0)) / det;
  r(0,1) = -(m(0,1) * m(2,2) - m(0,2) * m(2,1)) / det;
  r(1,1) =  (m(0,0) * m(2,2) - m(0,2) * m(2,0)) / det;
  r(2,1) = -(m(0,0) * m(2,1) - m(0,1) * m(2,0)) / det;
  r(0,2) =  (m(0,1) * m(1,2) - m(0,2) * m(1,1)) / det;
  r(1,2) = -(m(0,0) * m(1,2) - m(0,2) * m(1,0)) / det;
  r(2,2) =  (m(0,0) * m(1,1) - m(0,1) * m(1,0)) / det;

  // "translation part" - multiply the translation (on the left) by the inverse linear part
  r(0,3) = -(m(0,3) * r(0,0) + m(1,3) * r(0,1) + m(2,3) * r(0,2));
  r(1,3) = -(m(0,3) * r(1,0) + m(1,3) * r(1,1) + m(2,3) * r(1,2));
  r(2,3) = -(m(0,3) * r(2,0) + m(1,3) * r(2,1) + m(2,3) * r(2,2));
  // TODO: figure out why the next line casues problems
  //assert(isAffine(r) && norm2(Mat4() - m*r) < EPS2);
  return r;
}
//tested
inline Mat4 transpose(const Mat4& m)
{
    Mat4 r(0);
    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < 4; j++)
            r(i, j) = m(j, i);
    }
    return r;
}

inline Mat4 normalMatrix(const Mat4& m)
{
    Mat4 invm = inv(m);
    invm(0, 3) = invm(1, 3) = invm(2, 3) = 0;
    return transpose(invm);
}

inline Mat4 transFact(const Mat4& m)
{
    Mat4 r;
    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 3; j++)
            r(i, j) = m(i, j);
    }
}

inline Mat4 linFact(const Mat4& m)
{
    Mat4 r;
    r(0, 3) = m(0, 3);
    r(1, 3) = m(1, 3);
    r(2, 3) = m(2, 3);
}

inline Mat4 lookAt(const Vec3& pos, const Vec3& center, const Vec3& up)
{
    Vec3 z = normalize(center - pos);
    // up isn't the y axis, it's a vector in the yz plane
    // crossing up and the z axis produces the x axis
    Vec3 x = normalize(cross(z, up));
    Vec3 y = normalize(cross(x, z));
    Mat4 r;
    for(int i = 0; i < 3; i++)
    {
        r(0, i) = x[i];
        r(1, i) = y[i];
        r(2, i) = -z[i];
    }
    r(0, 3) = -dot(x, pos);
    r(1, 3) = -dot(y, pos);
    r(2, 3) =  dot(z, pos);
    return r;
} 


#endif
