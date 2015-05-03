#ifndef SCENEGRAPH_H
#define SCENEGRAPH_H

#include <stdio.h>
#include "geometry.h"
#include "rigtform.h"
#include "material.h"

// Arbitrary limit
#define MAX_CHILDREN 30
#define MAX_LAYER 10

enum NodeType {TRANSFORM, GEOMETRY, PICKABLEGEO};

class TransformNode 
{
public:
    TransformNode()
        :parent(NULL), parentToLocal(), childrenCount(0)
    {
        nt = TRANSFORM;
    }

    TransformNode(TransformNode *p, RigTForm& rbt)
        :parent(p), childrenCount(0)
    {
        // TODO: figure out copy constructor
        parentToLocal = rbt;
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

    RigTForm getRbt()
    {
        return parentToLocal;
    }

    void setRbt(RigTForm& rbt)
    {
        parentToLocal = rbt;
    }

    NodeType getNodeType()
    {
        return nt;
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

    int getNumChildren()
    {
        return childrenCount;
    }

    TransformNode* getParent()
    {
        return parent;
    }
    
    void setParent(TransformNode *p)
    {
        parent = p;
    }

    TransformNode* getChild(int i)
    {
        if(i < childrenCount)
        {
            return children[i];
        }else
        {
            fprintf(stderr, "Child index too high.\n");
            return NULL;
        }
    }

protected:
    NodeType nt;
    RigTForm parentToLocal;
    TransformNode *children[MAX_CHILDREN], *parent;;
    int childrenCount;
};

// TODO: change member names
class GeometryNode : public TransformNode
{
    
public:
    GeometryNode(TransformNode *p, RigTForm& rbt, Geometry *g,  Material *material)
        :TransformNode(p, rbt), geometry(g), m(material)
    {
        nt = GEOMETRY;
    }
    Geometry* getGeometry()
    {
        return geometry;
    }

    void setGeometry(Geometry *g)
    {
        geometry = g;
    }

    Material* getMaterial()
    {
        return m;
    }

    void setMaterial(Material *material)
    {
        m = material;
    }

    void draw(RigTForm modelViewRbt)
    {
        m->draw(geometry, modelViewRbt);
    }

protected:
    Geometry *geometry;
    Material *m;
};

class PickableGeoNode : public GeometryNode
{
public:
    PickableGeoNode(TransformNode *p, RigTForm& rbt, Geometry *g, Material *material,
                    Material *pm, int c)
        :GeometryNode(p, rbt, g, material), pickMaterial(pm), code(c)
    {
        nt = PICKABLEGEO;
    }

    Material* getPickMaterial()
    {
        return pickMaterial;
    }

    void setPickMaterial(Material *pm)
    {
        pickMaterial = pm;
    }

    int getCode()
    {
        return code;
    }

    void drawPicking(RigTForm modelViewRbt)
    {
        pickMaterial->draw(geometry, modelViewRbt);
    }

private:
    Material *pickMaterial;
    GLint code;
};

class Visitor
{
public:

    Visitor()
        :rbtCount(0)
    {}
    
    Visitor(RigTForm& rbt)
        :rbtCount(0)
    {
        viewRbt = rbt;
    }

    RigTForm getRbt()
    {
        return viewRbt;
    }

    void setRbt(RigTForm& rbt)
    {
        viewRbt = rbt;
    }

    bool pushRbt(RigTForm& rbt)
    {
        if(rbtCount == MAX_LAYER)
        {
            fprintf(stderr, "Too many RigTForm in rbtStack.\n");
            return false;
        }else if(rbtCount == 0)
        {
            rbtStack[rbtCount++] = rbt;
        }else
        {
            rbtStack[rbtCount] = rbt * rbtStack[rbtCount-1];
            rbtCount++;
        }
        return true;
    }

    bool popRbt()
    {
        if(rbtCount == 0)
        {
            fprintf(stderr, "No RigTForm on rbtStack.\n");
            return false;
        }else
            rbtCount--;
    }

    void visitNode(TransformNode *tn)
    {
        if(tn->getNodeType() == TRANSFORM)
        {
            //printf("Visiting transform node.\n");
            pushRbt(tn->getRbt());            
            for(int i = 0; i < tn->getNumChildren(); i++)
            {
                this->visitNode(tn->getChild(i));
            }
            popRbt();
            //printf("Exiting transform node.\n");
        }else if(tn->getNodeType() == GEOMETRY || tn->getNodeType() == PICKABLEGEO)
        {
            //printf("Visiting geometry node.\n");
            RigTForm modelViewRbt;
            if(rbtCount == 0)
            {
                modelViewRbt = viewRbt * tn->getRbt();
            }else
            {
                modelViewRbt = viewRbt * tn->getRbt() * rbtStack[rbtCount-1];
            }
            GeometryNode *gn = static_cast<GeometryNode*>(tn);
            gn->draw(modelViewRbt);
            //printf("Exiting geometry node.\n");
        }
    }

    void visitPickNode(TransformNode *tn)
    {
        if(tn->getNodeType() == TRANSFORM)
        {
            //printf("Visiting transform node.\n");
            pushRbt(tn->getRbt());            
            for(int i = 0; i < tn->getNumChildren(); i++)
            {
                this->visitPickNode(tn->getChild(i));
            }
            popRbt();
            //printf("Exiting transform node.\n");
        }else if(tn->getNodeType() == GEOMETRY)
        {
            // TODO: Render the object with the background color
            
        }else if(tn->getNodeType() == PICKABLEGEO)
        {
            //printf("Visiting pickable node.\n");
            RigTForm modelViewRbt;
            if(rbtCount == 0)
            {
                modelViewRbt = viewRbt * tn->getRbt();
            }else
            {
                modelViewRbt = viewRbt * tn->getRbt() * rbtStack[rbtCount-1];
            }
            PickableGeoNode *pn = static_cast<PickableGeoNode*>(tn);
            pn->drawPicking(modelViewRbt);
            //printf("Exiting pickable node.\n");
        }
    }

protected:
    RigTForm rbtStack[MAX_LAYER];
    RigTForm viewRbt;
    int rbtCount;
};

/*
struct RenderObject
{
    // TODO: add a scaling rbt?
    Geometry *geometry;
    RigTForm modelRbt;
    RigTForm modelViewRbt;
    ShaderState *st;


    RenderObject(Geometry *g, RigTForm& m, ShaderState *shaderstate)
    {
        geometry = g;
        modelRbt = m;
        st = shaderstate;
    }

    void calcModelView(RigTForm& viewRbt)
    {
        modelViewRbt = viewRbt * modelRbt;
    }

    void draw()
    {
        st->draw(geometry, modelViewRbt);
    }
};
*/
#endif
