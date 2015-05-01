#ifndef RIGTFORM_H
#define RIGTFORM_H

#include <math.h>
#include <assert.h>

#include "quat.h"


class RigTForm
{
    Vec3 t_;
    Quat r_;

public:
    RigTForm()
        : t_(0)
    {
        assert(norm2(Quat(1, 0, 0, 0) - r_) < EPS2);
    }
    // TODO: figure out if this assgnment is necessary
    RigTForm(const Vec3& t, const Quat& r)
    {
        t_ = Vec3(t[0], t[1], t[2]);
        r_ = Quat(r[0], r[1], r[2], r[3]);
    }

    explicit RigTForm(const Vec3& t)
    {
        t_ = Vec3(t[0], t[1], t[2]);
        assert(norm2(Quat(1, 0, 0, 0) - r_) < EPS2);
    }

    explicit RigTForm(const Quat& r)
    {
        r_ = Quat(r[0], r[1], r[2], r[3]);        
    }
    
    Vec3 getTranslation() const
    {        
        return t_;
    }

    Quat getRotation() const
    {
        return r_;
    }

    RigTForm& setTranslation(const Vec3& t)
    {
        t_ = Vec3(t[0], t[1], t[2]);
        return *this;
    }

    RigTForm& setRotation(const Quat& r)
    {
        r_ = Quat(r[0], r[1], r[2], r[3]);
        return *this;
    }

    // TODO: make sure the next two functions work
    Vec3 operator * (const Vec3& a) const
    {
        Vec3 r;
        r = r_*a + t_;
        return r;
    }

    RigTForm operator * (const RigTForm& a) const
    {
        RigTForm r;

        r.setTranslation(t_ + r_*a.getTranslation());
        r.setRotation(r_*a.getRotation());
        return r;
    }

    // TODO: crashes when center is already in the z axis
    static RigTForm lookAt(const Vec3& pos, const Vec3& center, const Vec3& up)
    {
        RigTForm r;
        Vec3 newCenter = normalize(center - pos);
        Vec3 rotAxis = normalize(cross(newCenter, Vec3(0, 0, -1)));
        float halfAngle = (float)acos(dot(newCenter, Vec3(0, 0, -1))) / 2.0f;

        float sinHalfAng = sin(halfAngle);
        Quat q(cos(halfAngle), sinHalfAng*rotAxis[0], sinHalfAng*rotAxis[1], sinHalfAng*rotAxis[2]);

        Vec3 newUp = q*up;
        Vec3 newX = normalize(cross(Vec3(0, 0, -1), newUp));
        halfAngle = (float)acos(dot(newX, Vec3(1, 0, 0))) / 2.0f;
        float sign = 1;
        // Not sure why this works
        if(newUp[0] < 0.0f)
            sign = -1;

        q = Quat(cos(halfAngle), 0, 0, sin(halfAngle)*sign) * q;
        Vec3 newPos = q*pos;
        r.setRotation(q);
        r.setTranslation(-newPos);
        return r;
    }
};

inline RigTForm inv(const RigTForm& tform)
{
    RigTForm r;
    Quat invr = inv(tform.getRotation());
    r.setTranslation(-(invr*tform.getTranslation()));
    r.setRotation(invr);
    return r;
}

inline RigTForm transFact(const RigTForm& tform)
{
    return RigTForm(tform.getTranslation());
}

inline RigTForm linFact(const RigTForm& tform)
{
    return RigTForm(tform.getRotation());
}

inline Mat4 rigTFormToMat(const RigTForm& tform)
{
    Mat4 r = quatToMat(tform.getRotation());
    Vec3 t = tform.getTranslation();
    r(0, 3) = t[0];
    r(1, 3) = t[1];
    r(2, 3) = t[2];
    return r;
}

// TODO
inline RigTForm interp(const RigTForm& src, const RigTForm& dest, float t)
{
    RigTForm r;

    return r;
}

#endif
