#pragma once
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
#include<mygeo/vec.h>

class MyWindow
{
public:
    MyWindow(int width, int height)
    {
        std::cout<<"width&height "<<width<<" , "<<height<<std::endl;
        initWindow(width,height);
    }
    GLFWwindow* window;
    float mousescrollVal=0.f;
    bool leftmousePressed=false;
    bool rightmousePressed=false;

    MyGeo::Vec2f leftDragVec{0,0};
    MyGeo::Vec2f rightDragVec{0,0};

    
    MyGeo::Vec2f leftStartPos{0,0};
    MyGeo::Vec2f rightStartPos{0,0};


    bool framebufferResized=false;
    void initWindow(int width, int height)
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetMouseButtonCallback(window,mousebutton_callback);
        glfwSetCursorPosCallback(window,cursor_position_callback);
    }

    static void mousebutton_callback(GLFWwindow* window, int button, int action, int mods)
    {
        auto mywindow = reinterpret_cast<MyWindow*>(glfwGetWindowUserPointer(window));
        if(button==GLFW_MOUSE_BUTTON_LEFT  && action==GLFW_PRESS)
        {
            mywindow->leftDragVec={0,0};
            double x,y;
            glfwGetCursorPos(window,&x,&y);
            mywindow->leftStartPos={(float)x,(float)y};
            mywindow->leftmousePressed=true;
        }
        if(button==GLFW_MOUSE_BUTTON_RIGHT  && action==GLFW_PRESS)
        {
            mywindow->rightDragVec={0,0};
            double x,y;
            glfwGetCursorPos(window,&x,&y);
            mywindow->rightStartPos={(float)x,(float)y};
            mywindow->rightmousePressed=true;
        }
        if(button==GLFW_MOUSE_BUTTON_LEFT && action==GLFW_RELEASE)
        {
            mywindow->leftmousePressed=false;
        }
        if(button==GLFW_MOUSE_BUTTON_RIGHT && action==GLFW_RELEASE)
        {
            mywindow->rightmousePressed=false;
        }

    }

    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
    {
        auto mywindow = reinterpret_cast<MyWindow*>(glfwGetWindowUserPointer(window));
        if(mywindow->leftmousePressed)
        {
            mywindow->leftDragVec=MyGeo::Vec2f{(float)xpos,(float)ypos}-mywindow->leftStartPos;
            mywindow->leftDragVec.y=-mywindow->leftDragVec.y;
        }
        if(mywindow->rightmousePressed)
        {
            mywindow->rightDragVec=MyGeo::Vec2f{(float)xpos,(float)ypos}-mywindow->rightStartPos;
            mywindow->rightDragVec.y=-mywindow->rightDragVec.y;
            mywindow->rightDragVec.x=-mywindow->rightDragVec.x;
        }
    }

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        auto mywindow = reinterpret_cast<MyWindow*>(glfwGetWindowUserPointer(window));
        mywindow->mousescrollVal+=yoffset;
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) 
    {
        auto mywindow = reinterpret_cast<MyWindow*>(glfwGetWindowUserPointer(window));
        mywindow->framebufferResized = true;
    }

    void cleanup()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }


};


