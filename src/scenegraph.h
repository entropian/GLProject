#ifndef SCENEGRAPH_H
#define SCENEGRAPH_H

#include "geometry.h"
#include "rigtform.h"

struct RenderObject
{
    // TODO: add a scaling rbt?
    Geometry *geometry;
    RigTForm modelRbt;
    RigTForm modelViewRbt;

    RenderObject(Geometry *g, RigTForm& m)
    {
        geometry = g;
        modelRbt = m;
    }

    void calcModelView(RigTForm& viewRbt)
    {
        modelViewRbt = viewRbt * modelRbt;
    }
};

#endif
