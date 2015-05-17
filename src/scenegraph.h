#ifndef SCENEGRAPH_H
#define SCENEGRAPH_H

#include <stdio.h>
#include "geometry.h"
#include "rigtform.h"
#include "material.h"

// Arbitrary limit
#define MAX_CHILDREN 30
#define MAX_LAYER 10
#define MAX_OBJECTS_ONSCREEN 50

static bool g_debugString = false;

enum NodeType {transformnode, geometrynode};

class TransformNode 
{
public:
    TransformNode()
        :parent(NULL), parentToLocal(), childrenCount(0)
    {
        nt = transformnode;
    }

    TransformNode(RigTForm& rbt)
        :parent(NULL), childrenCount(0)
    {
        // TODO: figure out copy constructor
        parentToLocal = rbt;
    }

    int getChildrenCount()
    {
        return childrenCount;
    }

    void setParent(TransformNode *p)
    {
        parent = p;
    }
    
    bool addChild(TransformNode *tn)
    {
        if(childrenCount == MAX_CHILDREN)
        {
            fprintf(stderr, "Children full.\n");
            return false;
        }
        children[childrenCount++] = tn;
        tn->setParent(this);
        return true;
    }

    RigTForm getRigidBodyTransform()
    {
        return parentToLocal;
    }

    void setRigidBodyTransform(RigTForm rbt)
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
                children[i]->setParent(NULL);
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

private:
    RigTForm parentToLocal;
    // TODO: is parent needed?
    TransformNode *children[MAX_CHILDREN], *parent;;
    int childrenCount;
};

// TODO: change member names
class GeometryNode : public TransformNode
{
    
public:
    GeometryNode(RigTForm& rbt, Geometry *g,  Material *material, bool c)
        :TransformNode(rbt), geometry(g), m(material), clickable(c), depthTest(true)
    {
        nt = geometrynode;
        scaleFactor = Vec3(1.0f, 1.0f, 1.0f);
    }
    Geometry* getGeometry()
    {
        return geometry;
    }

    void setGeometry(Geometry *g)
    {
        geometry = g;
    }

    Vec3 getScaleFactor()
    {
        return scaleFactor;
    }

    void setScaleFactor(Vec3 v)
    {
        scaleFactor = v;
    }

    Material* getMaterial()
    {
        return m;
    }

    void setMaterial(Material *material)
    {
        m = material;
    }

    bool getClickable()
    {
        return clickable;
    }

    void setClickable(bool c)
    {
        clickable = c;
    }

    bool getDepthTest()
    {
        return depthTest;
    }

    void setDepthTest(bool b)
    {
        depthTest = b;
    }

    void draw(RigTForm modelViewRbt)
    {
        if(depthTest == false)
            glDisable(GL_DEPTH_TEST);
        m->draw(geometry, modelViewRbt, scaleFactor);
        if(depthTest == false)
            glEnable(GL_DEPTH_TEST);
    }

    void overrideMatDraw(Material *overrideMat, RigTForm modelViewRbt)
    {
        if(depthTest == false)
            glDisable(GL_DEPTH_TEST);
        overrideMat->draw(geometry, modelViewRbt, scaleFactor);
        if(depthTest == false)
            glEnable(GL_DEPTH_TEST);
    }

private:
    Geometry *geometry;
    Material *m;
    Vec3 scaleFactor;
    bool clickable;  // Indicates whether the node can be selected by clicking
    bool depthTest;  // Indicates whether the node can be covered by other object
};

class Visitor
{
public:

    Visitor()
        :rbtCount(0), code(0)
    {}
    
    Visitor(RigTForm rbt)
        :rbtCount(0), code(0)
    {
        viewRbt = rbt;
    }

    RigTForm getViewTransform()
    {
        return viewRbt;
    }

    void setViewTransform(RigTForm& rbt)
    {
        viewRbt = rbt;
    }

    void visitNode(TransformNode *tn)
    {
        pushRbt(tn->getRigidBodyTransform());            
        if(tn->getNodeType() == transformnode)
        {
            if(g_debugString == true)
                printf("Visiting transform node.\n");

            for(int i = 0; i < tn->getNumChildren(); i++)
            {
                this->visitNode(tn->getChild(i));
            }

            if(g_debugString == true)
                printf("Exiting transform node.\n");
        }else if(tn->getNodeType() == geometrynode)
        {
            if(g_debugString == true)
                printf("Visiting geometry node.\n");
            RigTForm modelViewRbt;
            if(rbtCount == 0)
            {
                modelViewRbt = viewRbt;
            }else
            {
                modelViewRbt = viewRbt * rbtStack[rbtCount-1];
            }
            GeometryNode *gn = static_cast<GeometryNode*>(tn);
            gn->draw(modelViewRbt);

            for(int i = 0; i < gn->getNumChildren(); i++)
            {
                this->visitNode(gn->getChild(i));
            }
            if(g_debugString == true)
                printf("Exiting geometry node.\n");
        }
        popRbt();
    }

   
    void visitPickNode(TransformNode *tn, Material *overrideMat)
    {
        pushRbt(tn->getRigidBodyTransform());            
                    
        if(tn->getNodeType() == transformnode)
        {
            if(g_debugString == true)
                printf("Visiting transform node.\n");
            
            for(int i = 0; i < tn->getNumChildren(); i++)
            {
                this->visitPickNode(tn->getChild(i), overrideMat);
            }

            if(g_debugString == true)
                printf("Exiting transform node.\n");
        }else if(tn->getNodeType() == geometrynode)
        {
            if(g_debugString == true)
            printf("Visiting geometry node.\n");
            GeometryNode *gn = static_cast<GeometryNode*>(tn);
            
            RigTForm modelViewRbt;            
            if(rbtCount == 0)
                modelViewRbt = viewRbt;
            else
                modelViewRbt = viewRbt * rbtStack[rbtCount-1];

            if(gn->getClickable() == true)
            {

                // The corresponding geoNodes index for a node is code - 1
                // NOTE: Not sure if casting changes the pointer
                geoNodes[code] = static_cast<GeometryNode*>(tn);
                overrideMat->sendUniform1i("uCode", ++code);                
                gn->overrideMatDraw(overrideMat, modelViewRbt);
            }else
            {
                // TODO: render the object with background color or something like that
                // The back ground color is black. Setting uCode to 0 makes the color black
                overrideMat->sendUniform1i("uCode", 0);
                gn->overrideMatDraw(overrideMat, modelViewRbt);
            }

            for(int i = 0; i < gn->getNumChildren(); i++)
            {
                this->visitPickNode(gn->getChild(i), overrideMat);
            }
            if(g_debugString == true)
                printf("Exiting geometry node.\n");
        }
        popRbt();
    }

    GeometryNode* getClickedNode(unsigned char c)
    {
        if(c > code)
        {
            fprintf(stderr, "Invalid code.\n");
            return NULL;
        }
        return geoNodes[c];
    }


    
private:
    bool pushRbt(RigTForm rbt)
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
            rbtStack[rbtCount] = rbtStack[rbtCount-1] * rbt;
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
        return true;
    }

    void resetCode()
    {
        code = 0;
    }


    RigTForm rbtStack[MAX_LAYER];
    RigTForm viewRbt;
    int rbtCount;

    GeometryNode *geoNodes[MAX_OBJECTS_ONSCREEN];
    GLint code;
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
