#ifndef INPUT_H
#define INPUT_H

#include "scenegraph.h"
#include <stdio.h>
#include "material.h"
#include "mesh.h"



#include <GL/glew.h>
#if __GNUG__
#   include <GLFW/glfw3.h>
#else
#   include <GL/glfw3.h>
#endif


enum InputMode
{
    FPS_MODE,               // Move around the scene with FPS control
    PICKING_MODE,           // Let's the user select a selectable object
    OBJECT_MODE             // Let's the user move a selected object around     
};

// TODO: fix this ugly stuff below
extern const char* arrowVertSrc;
extern const char* flatFragSrc;
extern const char* pickVertSrc;
extern const char* pickFragSrc;

/*
  Accepts a mouse button or key press, makes changes to its members,
  Some members are exposed via public accesor methods.

 */
class InputHandler
{
public:
    InputHandler()
        :inputMode(FPS_MODE), pickModeClicked(false), pickedObj(NULL), pickedArrow(NULL),
        clickX(0.0), clickY(0.0), windowWidth(1280.0f), windowHeight(720.0f), arrow(NULL),
        arrowYMat(NULL), arrowZMat(NULL), arrowXMat(NULL), worldNode(NULL), pickMaterial(NULL)
    {}

    void initialize();
    void handleKey(GLFWwindow *window, int key, int scancode, int action, int mods);
    void handleMouseButton(GLFWwindow *window, int button, int action, int mods);
    void handleCursor(GLFWwindow* window, double x, double y);    

    Vec3 getMovementDir()
    {
        return movementDir;
    }

    InputMode getInputMode()
    {
        return inputMode;
    }
    
    RigTForm getViewTransform()
    {
        return viewRbt;
    }

    void setViewTransform(RigTForm rbt)
    {
        viewRbt = rbt;
    }
    
    TransformNode* getWorldNode()
    {
        return worldNode;
    }
    
    void setWorldNode(TransformNode *tn)
    {
        worldNode = tn;
    }           

private:
    void putArrowsOn(GeometryNode *gn);
    void removeArrows(GeometryNode *gn);
    void setArrowsClickable();
    void setArrowsUnclickable();
    void FPSModeKeyInput(GLFWwindow *window, int key, int scancode, int action, int mods);
    void ObjModeKeyInput(GLFWwindow *window, int key, int scancode, int action, int mods);
    void calcPickedObj();
    void calcPickedArrow();
    void updateArrowOrientation();
    
    Vec3 movementDir;
    // When inputMode == PICKING_MODE, nothing in the scene should be updated and re-rendered.
    InputMode inputMode;
    InputMode prevInputMode;
    bool pickModeClicked;
    double clickX, clickY;
    GeometryNode *pickedObj, *pickedArrow;
    RigTForm viewRbt;
    Mat4 projMat;
    float windowWidth, windowHeight;

    double cursorX, cursorY;

    // WTF is this all shit?
    Geometry *arrow;
    GeometryNode *arrowYNode, *arrowZNode, *arrowXNode;
    Material *arrowYMat, *arrowZMat, *arrowXMat;
    TransformNode *worldNode;
    Material *pickMaterial;
    RigTForm clickRbt;
    
};


#endif

