#ifndef RIGTFORM_H
#define RIGTFORM_H

#include <cmath>
#include <cassert>

#include "vec.h"
#include "mat.h"
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
    }

    RigTForm& setRotation(const Quat& r)
    {
        r_ = Quat(r[0], r[1], r[2], r[3]);
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

inline RigTForm interp(const RigTForm& src, const RigTForm& dest, float t)
{
    RigTForm r;

    return r;
}

#endif
