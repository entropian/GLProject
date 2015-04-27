#ifndef SCENEGRAPH_H
#define SCENEGRAPH_H

#include <stdio.h>
#include "geometry.h"
#include "rigtform.h"

// Arbitrary limit
#define MAX_CHILDREN 20

class TransformNode
{
public:
    TransformNode()
        :parent(NULL), rbt(), childrenCount(0)
    {}

    TransformNode(TransformNode *p, RigTForm& r)
        :parent(p), childrenCount(0)
    {
        // TODO: figure out copy constructor
        rbt = r;
    }

    int getChildrenCount()
    {
        return childrenCount;
    }

    bool addChild(TransformNode *tn)
    {
        if(childrenCount == 20)
        {
            fprintf(stderr, "Children full.\n");
            return false;
        }
        children[childrenCount++] = tn;
        return true;
    }

    bool removeChild(TransformNode *tn)
    {
        for(int i = 0; i < childrenCount; i++)
        {
            if(children[i] == tn)
            {
                int j;
                for(j = i; j < childrenCount - 1; j++)
                    children[j] = children[j+1];

                children[j] = NULL;
                childrenCount--;
                return true;
            }
        }

        fprintf(stderr, "No such child.\n");
        return false;
    }
        

private:
    RigTForm rbt;
    TransformNode *children[MAX_CHILDREN] = {NULL}, *parent;;
    int childrenCount;
};

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
