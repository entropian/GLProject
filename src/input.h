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


/*
  Accepts a mouse button or key press, makes changes to its members,
  Some members are exposed via public accesor methods.

 */
class InputHandler
{
public:
    InputHandler()
        :inputMode(FPS_MODE), pickModeClicked(false), pickedObj(NULL), pickedArrow(NULL)
    {}

    Vec3 getMovementDir()
    {
        return movementDir;
    }

    void setMovementDir(Vec3 v)
    {
        movementDir = v;
    }
    
    InputMode getInputMode()
    {
        return inputMode;
    }

    void setInputMode(InputMode im)
    {
        inputMode = im;
    }

    void setPickModeClicked(bool b)
    {
        pickModeClicked = b;
    }

    GeometryNode* getPickedObj()
    {
        return pickedObj;
    }

    void setPickedObj(GeometryNode *gn)
    {
        pickedObj = gn;
    }
    

    GeometryNode* getPickedArrow()
    {
        return pickedArrow;
    }

    void setPickedArrow(GeometryNode *gn)
    {
        pickedArrow = gn;
    }

    double getClickX()
    {
        return clickX;
    }

    void setClickX(double d)
    {
        clickX = d;
    }

    double getClickY()
    {
        return clickY;
    }

    void setClickY(double d)
    {
        clickY = d;
    }
        
    void setViewTransformation(RigTForm rbt)
    {
        viewRbt = rbt;
    }

    RigTForm getViewTransformation()
    {
        return viewRbt;
    }

    void setPickMaterial(Material *m)
    {
        pickMaterial = m;
    }

    TransformNode* getWorldNode()
    {
        return worldNode;
    }
    
    void setWorldNode(TransformNode *tn)
    {
        worldNode = tn;
    }    
    
    void setArrows(GeometryNode *y, GeometryNode *x, GeometryNode *z)
    {
        arrowYNode = y;
        arrowXNode = x;
        arrowZNode = z;
    }


            
    void FPSModeKeyInput(GLFWwindow *window, int key, int action);
    void calcPickedObj();
    void putArrowsOn(GeometryNode *gn);
    void removeArrows(GeometryNode *gn);
    void ObjModeKeyInput(int key, int action);

    
    void handleKey(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        if(key == GLFW_KEY_P && action == GLFW_PRESS)
        {
            if(inputMode == OBJECT_MODE)
                removeArrows(pickedObj);
            // TODO: Render the pickable objects to the back buffer.
            if(inputMode != PICKING_MODE)
            {
                printf("Switching to picking mode\n");

                prevInputMode = inputMode;
                inputMode = PICKING_MODE;  // Stops the rendering loop

                calcPickedObj();
            }else
            {
                inputMode = prevInputMode;
            } 
        }else if(key == GLFW_KEY_L && action == GLFW_PRESS)
        {
            if(inputMode == OBJECT_MODE)
            {
                removeArrows(pickedObj);
            }
            pickedObj = NULL;
            inputMode = FPS_MODE;
        }else if(inputMode == FPS_MODE)
        {
            FPSModeKeyInput(window, key, action);
        }else if(inputMode == OBJECT_MODE)
        {
            ObjModeKeyInput(key, action);
        }
    }

    void handleMouseButton(GLFWwindow *window, int button, int action, int mods)
    {

    }

private:
    /* Not sure how the cursor callback work yet
    double cursorX;
    double cursorY;
    */

    Vec3 movementDir;
    // When g_inputMode == PICKING_MODE, nothing in the scene should be updated and re-rendered.
    InputMode inputMode;
    InputMode prevInputMode;
    bool pickModeClicked;
    double clickX, clickY;
    GeometryNode *pickedObj, *pickedArrow;
    RigTForm viewRbt;
    int windowWidth = 800, windowHeight = 600;

    // placeholder. actual cursorX and cursorY are still calculated in main.cpp
    double cursorX, cursorY;

    // WTF is this shit?
    GeometryNode *arrowYNode, *arrowZNode, *arrowXNode;

    // More temporary BS
    TransformNode *worldNode;
    Material *pickMaterial;
    
};


#endif

