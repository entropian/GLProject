#ifndef INPUT_H
#define INPUT_H

#include "scenegraph.h"
#include <stdio.h>
#include "material.h"



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

extern const char* basicVertSrc;
extern const char* flatFragSrc;
extern const char* pickVertSrc;
extern const char* pickFragSrc;
extern GLfloat* readFromObj(const char* fileName, int *numVertices);

/*
  Accepts a mouse button or key press, makes changes to its members,
  Some members are exposed via public accesor methods.

 */
class InputHandler
{
public:
    InputHandler()
        :inputMode(FPS_MODE), pickModeClicked(false), pickedObj(NULL), pickedArrow(NULL)
    {
    }

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
    Vec3 movementDir;
    // When g_inputMode == PICKING_MODE, nothing in the scene should be updated and re-rendered.
    InputMode inputMode;
    InputMode prevInputMode;
    bool pickModeClicked;
    double clickX, clickY;
    GeometryNode *pickedObj, *pickedArrow;
    RigTForm viewRbt;
    int windowWidth = 800, windowHeight = 600;

    double cursorX, cursorY;

    // WTF is this all shit?
    Geometry *arrow;
    GeometryNode *arrowYNode, *arrowZNode, *arrowXNode;
    Material *arrowYMat, *arrowZMat, *arrowXMat;
    TransformNode *worldNode;
    Material *pickMaterial;
    
};


#endif

