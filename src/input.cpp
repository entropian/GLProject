#include "input.h"

// Places coordinate system arrows on an object
void InputHandler::putArrowsOn(GeometryNode *gn)
{

    gn->addChild(arrowYNode);
    gn->addChild(arrowZNode);
    gn->addChild(arrowXNode);
}

// Maintain the coordinate system arrows' orientation
void InputHandler::updateArrowOrientation()
{
    RigTForm counterRotation;
    TransformNode *tn = pickedObj;
    while(tn != worldNode)
    {
        counterRotation = linFact(tn->getRigidBodyTransform()) * counterRotation;
        tn = tn->getParent();
    }
    counterRotation = inv(counterRotation);
    arrowYNode->setRigidBodyTransform(counterRotation);
    arrowZNode->setRigidBodyTransform(counterRotation * RigTForm(Quat::makeXRotation(-90.0f)));
    arrowXNode->setRigidBodyTransform(counterRotation * RigTForm(Quat::makeZRotation(-90.0f)));    
}

// Removess the coordinate systems arrows on an object
void InputHandler::removeArrows(GeometryNode *gn)
{
    gn->removeChild(arrowYNode);    
    gn->removeChild(arrowZNode);
    gn->removeChild(arrowXNode);
    
    // Restore the arrows' default orientation
    arrowYNode->setRigidBodyTransform(RigTForm());
    arrowZNode->setRigidBodyTransform(RigTForm(Quat::makeXRotation(-90.0f)));
    arrowXNode->setRigidBodyTransform(RigTForm(Quat::makeZRotation(-90.0f)));    
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
    // Loads the arrow model
    Mesh arrowMesh;
    arrowMesh.loadOBJFile("arrow.obj");
    arrow = arrowMesh.produceGeometry(PNX);

    projMat = Mat4::makePerspective(60.0f, windowWidth/windowHeight, 0.1f, 30.0f);

    // Initialize arrow materials and picking material
    arrowYMat = new Material(arrowVertSrc, flatFragSrc, "arrowYMaterial");
    arrowYMat->sendUniform3f("uColor", Vec3(0.0f, 0.0f, 1.0f));
    arrowYMat->sendUniformMat4("uProjMat", projMat);

    arrowZMat = new Material(arrowVertSrc, flatFragSrc, "arrowZMaterial");
    arrowZMat->sendUniform3f("uColor", Vec3(0.0f, 1.0f, 0.0f));
    arrowZMat->sendUniformMat4("uProjMat", projMat);

    arrowXMat = new Material(arrowVertSrc, flatFragSrc, "arrowXMaterial");
    arrowXMat->sendUniform3f("uColor", Vec3(1.0f, 0.0f, 0.0f));
    arrowXMat->sendUniformMat4("uProjMat", projMat);

    pickMaterial = new Material(pickVertSrc, pickFragSrc, "PickMaterial");
    pickMaterial->sendUniformMat4("uProjMat", projMat);

    // Initialize arrow GeometryNodes
    RigTForm modelRbt = RigTForm(Vec3(0.0f, 0.0f, 0.0f));
    arrowYNode = new GeometryNode(arrow, arrowYMat, modelRbt, false);
    arrowYNode->setDepthTest(false);

    modelRbt = RigTForm(Quat::makeZRotation(-90.0f));
    arrowXNode = new GeometryNode(arrow, arrowXMat, modelRbt, false);
    arrowXNode->setDepthTest(false);

    modelRbt = RigTForm(Quat::makeXRotation(-90.0f));
    arrowZNode = new GeometryNode(arrow, arrowZMat, modelRbt, false);
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

// Determines which object has been clicked on
void InputHandler::calcPickedObj()
{
    // TODO: Doesn't work well when objects are in the edge of the window
    // or when I'm clicking near the edge of the object    
    
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
    }

    if(pickModeClicked == true && inputMode == PICKING_MODE)
    {
        // Determine which object is picked
        unsigned char pixel[3];
        glReadPixels((GLint)cursorX, (GLint)(windowHeight - cursorY), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &pixel);
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
    RigTForm rbt;
    switch(key)
    {
    case GLFW_KEY_A:
        rbt = RigTForm(Quat::makeYRotation(-5.0f));
        break;
    case GLFW_KEY_D:
        rbt = RigTForm(Quat::makeYRotation(5.0f));
        break;
    case GLFW_KEY_W:
        rbt = RigTForm(Quat::makeXRotation(-5.0f));
        break;
    case GLFW_KEY_S:
        rbt = RigTForm(Quat::makeXRotation(5.0f));
        break;        
    }    

    RigTForm rotation, counterRotation; 
    TransformNode *tn = pickedObj;
    while(tn != worldNode)
    {
        rotation = linFact(tn->getRigidBodyTransform()) * rotation;
        tn = tn->getParent();
    }
    counterRotation = inv(rotation);
    // TODO: investigate how the quaternion stops being a unit quaternion
    RigTForm newRbt = pickedObj->getRigidBodyTransform() * counterRotation * rbt * rotation;
    newRbt.setRotation(normalize(newRbt.getRotation()));
    pickedObj->setRigidBodyTransform(newRbt);
    updateArrowOrientation();
}

// Determine which coordinate system arrow has been clicked on
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
    glReadPixels((GLint)cursorX, (GLint)(windowHeight - cursorY), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &pixel);
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
                // can't put arrows on nothing
                if(pickedObj != NULL)
                    updateArrowOrientation();
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
    case GLFW_KEY_ESCAPE:
        if(pickedArrow != NULL && inputMode == OBJECT_MODE)
        {
            pickedArrow = NULL;
            pickedObj->setRigidBodyTransform(clickRbt);           
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

            if(pickedArrow != NULL)
            {
                clickX = cursorX;
                clickY = cursorY;
                // clickRbt is only set here
                clickRbt = pickedObj->getRigidBodyTransform();
            }
 
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

// Handles cursor movement
void InputHandler::handleCursor(GLFWwindow* window, double x, double y)
{
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED && inputMode == FPS_MODE)
    {
        float dx = (float)(x - cursorX);
        float dy = (float)(y - cursorY);

        RigTForm tform = RigTForm(Quat::makeYRotation(dx * 0.05f)) * RigTForm(Quat::makeXRotation(dy * 0.05f));
        viewRbt = tform * viewRbt;
        // Keep the camera upright
        // Rotate the camera around its z axis so that its y axis is
        // in the plane defined by its z axis and world y axis.
        Vec3 newUp = viewRbt.getRotation() * Vec3(0, 1, 0);
        Vec3 newX;
        if(abs(dot(newUp, Vec3(0.0f, 0.0f, -1.0f))) < 0.999f)
        {
            newX = normalize(cross(Vec3(0, 0, -1), newUp));
            float halfAngle = (float)acos(dot(newX, Vec3(1, 0, 0))) / 2.0f;
            float sign = 1;
            // Not sure why this works
            if(newUp[0] < 0.0f)
                sign = -1;
            Quat q(cos(halfAngle), 0, 0, sin(halfAngle)*sign);
            tform = RigTForm(Vec3(0, 0, 0), q);
            viewRbt = tform * viewRbt;;
        }

        /*
        // Report camera position in world space
        RigTForm invView = inv(g_view);
        Vec3 trans = invView.getTranslation();        
        std::cout << "Camera pos: " << trans[0] << " " << trans[1] << " " << trans[2] << "\n";
        */

    }else if(pickedArrow != NULL && inputMode == OBJECT_MODE)
    {
        Vec4 axis;
        // w is set to zero, since axis is a vector
        if(pickedArrow == arrowYNode)
        {
            axis = Vec4(0.0f, 1.0f, 0.0f, 0.0f);
        }else if(pickedArrow == arrowZNode)
        {
            axis = Vec4(0.0f, 0.0f, -1.0f, 0.0f);
        }else if(pickedArrow == arrowXNode)
        {
            axis = Vec4(1.0f, 0.0f, 0.0f, 0.0f);
        }

        Mat4 viewMat = rigTFormToMat(viewRbt);
        Vec4 clipAxis = projMat * viewMat * axis;
        Vec3 clipVec = normalize(Vec3(clipAxis[0], clipAxis[1], 0.0f));
        float distance = dot(clipVec, Vec3(x - (float)cursorX, (float)cursorY - y, 0.0f));

        

        // TODO: make 80.0f into a class member
        Vec3 translation = Vec3(axis[0], axis[1], axis[2]) * (distance / 80.0f);
        //RigTForm newRbt(Vec3(axis[0], axis[1], axis[2]) * (distance / 80.0f));
        // Ensures that movement is always along world axes.
        RigTForm counterRotation;
        TransformNode *tn = pickedObj;
        while(tn != worldNode)
        {
            counterRotation = linFact(tn->getRigidBodyTransform()) * counterRotation;
            tn = tn->getParent();
        }
        counterRotation = inv(counterRotation);
        translation = counterRotation * translation;
        RigTForm newRbt(translation);
        pickedObj->setRigidBodyTransform(pickedObj->getRigidBodyTransform() * newRbt);
        updateArrowOrientation();
    }
    
    cursorX = x;
    cursorY = y;
}
