#ifndef INPUT_H
#define INPUT_H

#if __GNUG__
#include <GLFW/glfw3.h>
#else
#include <GL/glfw3.h>


#include <iostream>
#include <stdio.h>
#include "scenegraph.h"

enum InputMode{
    FPS_MODE,               // Move around the scene with FPS control
    PICKING_MODE,           // Let's the user select a selectable object
    OBJECT_MODE             // Let's the user move a selected object around     
};

class InputHandler
{
public:
    InputHandler()
        :inputMode(FPS_MODE), pickModeClicked(false), pickedObj(NULL), pickedArrow(NULL),
    {}

    Vec3 getMovementDir()
    {
        return movementDir;
    }

    void setMovementDir(Vec3& v)
    {
        movementDir = v;
    }

    InputMode getInputMode()
    {
        return inputMode;
    }

    GeometryNode* getPickedObj()
    {
        return pickedObj;
    }

    void handleKey(GLFWwindow *window, int key, int scancode, int action, int mods)
    {

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
    InputMode inputMode;
    inputMode prevInputMode;
    bool pickModeClicked;
    double clickX, clickY;
    GeometryNode *pickedObj, *pickedArrow;
};


#endif
