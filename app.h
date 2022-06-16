#pragma once
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
#include<iostream>
#include"my_window.h"
#include"my_device.h"
#include"my_swapchain.h"
#include"my_appdata.h"
#include"my_buffer.h"
#include"my_pipeline.h"
#include"my_renderer.h"
#include"cornell.h"

class App
{
public:
    int width=800, height=600;
    MyWindow mywindow{width,height};
    MyDevice mydevice{mywindow};
    MySwapChain myswapChain{mydevice};
    MyAppData myappdata{myswapChain};
    MyPipeline mypipeline{myappdata};
    MyRenderer renderer{mypipeline};
    App()
    {
        
    }

    void run()
    {
        renderer.mainLoop();
        cleanup();
    }

    void cleanup()
    {
        renderer.cleanup();
    }

private:

};

