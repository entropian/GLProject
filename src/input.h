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

    GeometryNode* getArrowY()
    {
        return arrowYNode;
    }

    GeometryNode* getArrowZ()
    {
        return arrowZNode;
    }

    GeometryNode* getArrowX()
    {
        return arrowXNode;
    }    

    double getCursorX()
    {
        return cursorX;
    }

    void setCursorX(double x)
    {
        cursorX = x;
    }
        
    double getCursorY()
    {
        return cursorY;
    }        

    void setCursorY(double y)
    {
        cursorY = y;
    }
        
    void FPSModeKeyInput(GLFWwindow *window, int key, int action);
    void calcPickedObj();
    void putArrowsOn(GeometryNode *gn);
    void removeArrows(GeometryNode *gn);
    void setArrowsClickable();
    void setArrowsUnclickable();
    void ObjModeKeyInput(int key, int action);
    void calcPickedArrow();
    

    
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
        if (button != GLFW_MOUSE_BUTTON_LEFT)
            return;

        if(inputMode == FPS_MODE)
        {
            if (action == GLFW_PRESS)
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                glfwGetCursorPos(window, &cursorX, &cursorY);
            }
            else
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }else if(inputMode == PICKING_MODE)
        {
            if(action == GLFW_PRESS)
            {
                glfwGetCursorPos(window, &cursorX, &cursorY);
            }
            else if(action == GLFW_RELEASE)
            {
                // TODO: Get the mouse coordinate, and determine which object the user selected.
                pickModeClicked = true;
            }
        }else if(inputMode == OBJECT_MODE)
        {

            if(action == GLFW_PRESS)
            {
                // TODO: draw a frame to backbuffer for picking
                if(pickedArrow == NULL)
                {

                    setArrowsClickable();
                    calcPickedArrow();

                    if(pickedArrow == arrowYNode)
                        printf("Y arrow.\n");
                    else if(pickedArrow == arrowXNode)
                        printf("X arrow.\n");
                    else if(pickedArrow == arrowZNode)
                        printf("Z arrow.\n");

                    // Code for dragging object around

                }else
                {
                    /*
                      double cursorX, cursorY; // not sure
                      glfwGetCursorPos(window, &cursorX, &cursorY);
                      inputHandler.setCursorX(cursorX);
                      inputHandler.setCursorY(cursorY);
                    */
                }

            }else if(action == GLFW_RELEASE)
            {
                // TODO: use mouse release coordinate to determine
                // if one of the arrows was clicked
                if(pickedArrow != NULL)
                {
                    setArrowsUnclickable();
                    pickedArrow =NULL;
                }
            }
        }
    }

    void handleCursor(GLFWwindow* window, double x, double y)
    {
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        {
            float dx = (float)(x - cursorX);
            float dy = (float)(y - cursorY);


            RigTForm tform = RigTForm(Quat::makeYRotation(dx * 0.5f)) * RigTForm(Quat::makeXRotation(dy * 0.5f));
            viewRbt = tform * viewRbt;
            // Keep the camera upright
            // Rotate the camera around its z axis so that its y axis is
            // in the plane defined by its z axis and world y axis.
            Vec3 newUp = viewRbt.getRotation() * Vec3(0, 1, 0);
            Vec3 newX;
            if(abs(dot(newUp, Vec3(0.0f, 0.0f, -1.0f))) > 0.999f)
                newX = Vec3(0.0f, 0.0f, -1.0f);
            else
                newX = normalize(cross(Vec3(0, 0, -1), newUp));
            float halfAngle = (float)acos(dot(newX, Vec3(1, 0, 0))) / 2.0f;
            float sign = 1;
            // Not sure why this works
            if(newUp[0] < 0.0f)
                sign = -1;
            Quat q(cos(halfAngle), 0, 0, sin(halfAngle)*sign);
            tform = RigTForm(Vec3(0, 0, 0), q);
            viewRbt = tform * viewRbt;;

            // Report camera position in world space
            /*
              RigTForm invView = inv(g_view);
              Vec3 trans = invView.getTranslation();        
              std::cout << "Camera pos: " << trans[0] << " " << trans[1] << " " << trans[2] << "\n";
            */
        

        }
        cursorX = x;
        cursorY = y;
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

