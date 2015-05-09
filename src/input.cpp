#include "input.h"

void InputHandler::putArrowsOn(GeometryNode *gn)
{
    gn->addChild(arrowYNode);
    gn->addChild(arrowZNode);
    gn->addChild(arrowXNode);
}

void InputHandler::removeArrows(GeometryNode *gn)
{
    gn->removeChild(arrowYNode);
    gn->removeChild(arrowZNode);
    gn->removeChild(arrowXNode);
}

void InputHandler::setArrowsClickable()
{
    arrowYNode->setClickable(true);
    arrowZNode->setClickable(true);
    arrowXNode->setClickable(true);    
}

void InputHandler::setArrowsUnclickable()
{
    arrowYNode->setClickable(false);
    arrowZNode->setClickable(false);
    arrowXNode->setClickable(false);    
}

void InputHandler::FPSModeKeyInput(GLFWwindow *window, int key, int action)
{
       if(action == GLFW_PRESS)
       {
            int i = -1;
            float component = 0;
            switch(key)
            {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
            case GLFW_KEY_A: // left
                i = 0;
                component = 1.0f;
                break;
            case GLFW_KEY_D: // right
                i = 0;
                component = -1.0f;
                break;
            case GLFW_KEY_W: // forward
                i = 2;
                component = 1.0f;
                break;
            case GLFW_KEY_S: // backward
                i = 2;
                component = -1.0f;
                break;
            default:
                break;
            }
        
            if(i != -1)
            {
                if(movementDir[i] == 0.0f)
                    movementDir[i] = component;
                else if(abs(component - movementDir[i]) > abs(component))
                    movementDir[i] = 0.0f;
            }

            if(norm2(movementDir) != 0.0f)
                movementDir = normalize(movementDir);
        
        }else if(action == GLFW_RELEASE)
        {
            // This is a reverse of the above code in if(action == GLFW_PRESS)
            // with component taking the opposite value in each case
            // I treat directioanl key release as pressing a key of the opposite direction
            int i = -1;
            float component = 0.0f;
            switch(key)
            {
            case GLFW_KEY_A: // left
                i = 0;
                component = -1.0f;
                break;
            case GLFW_KEY_D: // right
                i = 0;
                component = 1.0f;
                break;
            case GLFW_KEY_W: // forward
                i = 2;
                component = -1.0f;
                break;
            case GLFW_KEY_S: // backward
                i = 2;
                component = 1.0f;
                break;
            default:
                break;
            }

            if(i != -1)
            {
                if(movementDir[i] == 0.0f)
                    movementDir[i] = component;
                else if(abs(component - movementDir[i]) > abs(component))
                    movementDir[i] = 0.0f;
            }
            if(norm2(movementDir) != 0.0f)
                movementDir = normalize(movementDir);

        }
}

void InputHandler::calcPickedObj()
{    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Visitor visitor(viewRbt);
    // Waits for a mouse click or the p key being pressed
    visitor.visitPickNode(worldNode, pickMaterial);
    
    // glReadPixels() actually reads from the back buffer
    // so don't bother swapping the buffer
    //glfwSwapBuffers(window);


    // Thread safety?
    // Waits for a mouse click or the p key being pressed
    pickModeClicked = false;
    while(pickModeClicked == false && inputMode == PICKING_MODE)
    {
        glfwWaitEvents();
        //glfwPollEvents();
    }

    if(inputMode == PICKING_MODE)
    {
        // Determine which object is picked
        unsigned char pixel[3];
        glReadPixels(cursorX, windowHeight - cursorY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &pixel);
        printf("pixel[0] == %d\n", pixel[0]);

        pickedObj = visitor.getClickedNode(pixel[0] - 1);

        // The rendering loop resumes
        // OBJECT_MODE can only be entered through here
        if(pickedObj != NULL)
        {
            inputMode = OBJECT_MODE;
            putArrowsOn(pickedObj);
        }else
            inputMode = prevInputMode;
    }else
    {

    }
}

void InputHandler::ObjModeKeyInput(int key, int action)
{
    Vec3 tmp;
    switch(key)
    {
    case GLFW_KEY_A:
        tmp[0] = -0.1f;
        break;

    case GLFW_KEY_D:
        tmp[0] = 0.1f;
        break;

    case GLFW_KEY_W:
        tmp[2] = -0.1f;
        break;

    case GLFW_KEY_S:
        tmp[2] = 0.1f;
        break;

    case GLFW_KEY_Q:
        tmp[1] = 0.1f;
        break;

    case GLFW_KEY_E:
        tmp[1] = -0.1f;
        break;
    default:
        break;
    }

    // TODO: convert world space transformation into object space
    RigTForm trans(tmp);
    pickedObj->setRigidBodyTransform(trans * pickedObj->getRigidBodyTransform());
}

void InputHandler::calcPickedArrow()
{
    printf("picking arrow\n");
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Visitor visitor(viewRbt);
    // Draws the picking frame to the backbuffer
    visitor.visitPickNode(worldNode, pickMaterial);

    // Determine which object is picked
    unsigned char pixel[3];
    glReadPixels(cursorX, windowHeight - cursorY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &pixel);
    printf("pixel[0] == %d\n", pixel[0]);

    pickedArrow = visitor.getClickedNode(pixel[0] - 1);

    if((pickedArrow == arrowYNode || pickedArrow == arrowZNode) || pickedArrow == arrowXNode)
    {
        clickX = cursorX;
        clickY = cursorY;
    }
    else
        pickedArrow = NULL;

}
