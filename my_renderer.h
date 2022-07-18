#pragma once
#include"my_pipeline.h"
// #include"my_event.h"
// #include<chrono>
// #include<mygeo/geo.h>
// #include<functional>
// #include"time/mytime.h"
// #include"cornell.h"
#include"my_model.h"
// #include"geometry/transforms.h"

// #include<imgui.h>
// #include <backends/imgui_impl_glfw.h>
// #include <backends/imgui_impl_vulkan.h>
// static constexpr int FrameRate=30;
// static constexpr int FrameInterval=1000/FrameRate;
static float mycolor[4]={0.5,0.5,0.5,1.0};


class MyRenderer
{
    
public:
    MyPipeline& scenePipeline;//graphics
    MyPipeline& shadowPipeline;//shadow
    MyDevice& mydevice;
    MySwapChain& myswapChain;
    Dispatcher& eventDispatcher;

    std::vector<VkCommandBuffer> commandBuffers;
    
    size_t currentFrame = 0;
static constexpr int MAX_FRAMES_IN_FLIGHT=2;

    MyRenderer(Dispatcher& dispatcher, MyPipeline& scenePipeline,MyPipeline& shadowPipeline):scenePipeline{scenePipeline},shadowPipeline{shadowPipeline},mydevice{scenePipeline.mydevice},myswapChain{scenePipeline.myswapChain},eventDispatcher{dispatcher}
    {
        eventDispatcher.subscribe(WindowResized_Event,[this](const Event& event){onEvent(event);});
        init();
    }

    void init()
    {
        allocateCommandBuffers();
    }

    void endFrame(uint32_t imageIndex);

    void onEvent(const Event& event);

    void recreateSwapChain();

    void reboot();

    void clear();

    void cleanup();
    virtual void allocateCommandBuffers();

    uint32_t startFrame();

    void beginCommandBuffer(VkCommandBuffer cmdbuffer);
};