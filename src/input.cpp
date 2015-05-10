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

void InputHandler::initialize()
{
    int numVertices;
    GLfloat *arrow_verts = readFromObj("arrow.obj", &numVertices);
    arrow = new Geometry(arrow_verts, numVertices);
    free(arrow_verts);

    Mat4 proj = Mat4::makeProjection(60.0f, 800.0f/600.0f, 0.1f, 30.0f);
    proj = transpose(proj);

    arrowYMat = new Material(basicVertSrc, flatFragSrc);
    arrowYMat->sendUniform3v("uColor", Vec3(0.0f, 0.0f, 1.0f));
    arrowYMat->sendUniformMat4("uProjMat", proj);

    arrowZMat = new Material(basicVertSrc, flatFragSrc);
    arrowZMat->sendUniform3v("uColor", Vec3(0.0f, 1.0f, 0.0f));
    arrowZMat->sendUniformMat4("uProjMat", proj);

    arrowXMat = new Material(basicVertSrc, flatFragSrc);
    arrowXMat->sendUniform3v("uColor", Vec3(1.0f, 0.0f, 0.0f));
    arrowXMat->sendUniformMat4("uProjMat", proj);


    pickMaterial = new Material(pickVertSrc, pickFragSrc);
    pickMaterial->sendUniformMat4("uProjMat", proj);

    RigTForm modelRbt = RigTForm(Vec3(0.0f, 0.0f, 0.0f));
    arrowYNode = new GeometryNode(modelRbt, arrow, arrowYMat, false);
    arrowYNode->setDepthTest(false);

    modelRbt = RigTForm(Quat::makeZRotation(-90.0f));
    arrowXNode = new GeometryNode(modelRbt, arrow, arrowXMat, false);
    arrowXNode->setDepthTest(false);

    modelRbt = RigTForm(Quat::makeXRotation(-90.0f));
    arrowZNode = new GeometryNode(modelRbt, arrow, arrowZMat, false);
    arrowZNode->setDepthTest(false);

}

void InputHandler::FPSModeKeyInput(GLFWwindow *window, int key, int scancode, int action, int mods)
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
    visitor.visitPickNode(worldNode, pickMaterial);
    
    // glReadPixels() actually reads from the back buffer
    // so don't bother swapping the buffer
    //glfwSwapBuffers(window);


    // Thread safety?
    // Waits for a mouse click or the p key being pressed
    // pickModeClicked is only set to false here
    pickModeClicked = false;
    while(pickModeClicked == false && inputMode == PICKING_MODE)
    {
        glfwWaitEvents();
        //glfwPollEvents();
    }

    if(pickModeClicked == true && inputMode == PICKING_MODE)
    {
        // Determine which object is picked
        unsigned char pixel[3];
        glReadPixels(cursorX, windowHeight - cursorY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &pixel);
        pickedObj = visitor.getClickedNode(pixel[0] - 1);

        // The rendering loop resumes
        // OBJECT_MODE can only be entered through here
        if(pickedObj != NULL)
        {
            inputMode = OBJECT_MODE;
            putArrowsOn(pickedObj);
        }else
        {
            // Reentry to previous input mode
            inputMode = prevInputMode;

            if(inputMode == OBJECT_MODE)
                putArrowsOn(pickedObj);
        }
    }
}
void InputHandler::ObjModeKeyInput(GLFWwindow *window, int key, int scancode, int action, int mods)
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
    pickedArrow = visitor.getClickedNode(pixel[0] - 1);
    
    // pickedArrow is also used as flag to signal to handleCursor() that an object is being dragged
    if((pickedArrow == arrowYNode || pickedArrow == arrowZNode) || pickedArrow == arrowXNode)
    {
        clickX = cursorX;
        clickY = cursorY;
    }
    else
        pickedArrow = NULL;

}

void InputHandler::handleKey(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    switch(key)
    {
    case GLFW_KEY_P:
        if(action == GLFW_PRESS)
        {
            // Clean up for exiting from other input modes
            // Put it in a function?
            if(inputMode == OBJECT_MODE)
                removeArrows(pickedObj);
            
            if(inputMode != PICKING_MODE)
            {
                prevInputMode = inputMode;
                inputMode = PICKING_MODE;
                calcPickedObj();
            }else
            {
                inputMode = prevInputMode;
            }
        }
        break;
    case GLFW_KEY_L:
        if(action == GLFW_PRESS)
        {
            if(inputMode == OBJECT_MODE)
            {
                removeArrows(pickedObj);                
                pickedObj = NULL;
            }
            inputMode = FPS_MODE;
        }
        break;
    default:
        switch(inputMode)
        {
        case FPS_MODE:
            FPSModeKeyInput(window, key, scancode, action, mods);
            break;
        case OBJECT_MODE:
            ObjModeKeyInput(window, key, scancode, action, mods);
            break;
        }
    }
}

void InputHandler::handleMouseButton(GLFWwindow *window, int button, int action, int mods)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return;

    switch(inputMode)
    {
    case FPS_MODE:
        if(action == GLFW_PRESS)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            // Not sure if this line is needed
            glfwGetCursorPos(window, &cursorX, &cursorY);
        }else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        break;
    case PICKING_MODE:
        if(action == GLFW_RELEASE)
        {
            // To signal to calcPickedObj() that a left click has occured
            pickModeClicked = true;
        }
        break;
    case OBJECT_MODE:
        if(action == GLFW_PRESS)
        {                
            setArrowsClickable();
            pickedArrow = NULL;
            calcPickedArrow();

            if(pickedArrow == arrowYNode)
                printf("Y arrow.\n");
            else if(pickedArrow == arrowXNode)
                printf("X arrow.\n");
            else if(pickedArrow == arrowZNode)
                printf("Z arrow.\n");
        }else
        {
            setArrowsUnclickable();
            pickedArrow = NULL;
        }
        break;
    }
}


void InputHandler::handleCursor(GLFWwindow* window, double x, double y)
{
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED && inputMode == FPS_MODE)
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
        

    }else if(pickedArrow != NULL && inputMode == OBJECT_MODE)
    {
        // TODO: object dragging code
    }
    cursorX = x;
    cursorY = y;
}
