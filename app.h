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
    ExtModel extmodel{mydevice,myswapChain,"../resources/teapot.obj"};
    ShapeModel rect{mydevice,myswapChain,GeoShape::Rect,{10.f,10.f}};
    App()
    {
        
    }

    void run()
    {
        mainLoop();
        cleanup();
    }

    void cleanup()
    {
        renderer.cleanup();
    }

//At a high level, rendering a frame in Vulkan consists of a common set of steps:
// Wait for the previous frame to finish
// Acquire an image from the swap chain
// Record a command buffer which draws the scene onto that image
// Submit the recorded command buffer
// Present the swap chain image
    virtual void renderFrame() 
    {
        uint32_t imageIndex=renderer.startFrame();
        updateFrameData(imageIndex);
        recordCommand(renderer.commandBuffers[renderer.currentFrame],imageIndex);
        renderer.endFrame(imageIndex);
    }
    virtual void mainLoop()
    {
        while(!glfwWindowShouldClose(mydevice.mywindow.window))
        {
            glfwPollEvents();
            renderFrame();
        }
        vkDeviceWaitIdle(mydevice.device);
    }
    virtual void recordCommand(VkCommandBuffer cmdBuffer, int imageIndex)
    {
        vkResetCommandBuffer(cmdBuffer,VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        renderer.startRecord(cmdBuffer,myswapChain.framebuffers[imageIndex],mypipeline.graphicsPipeline);
        extmodel.bind(cmdBuffer);
        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mypipeline.pipelineLayout, 0, 1, &myappdata.descriptorSets[imageIndex], 0, nullptr);
        extmodel.draw(cmdBuffer,extmodel.indices.size());
        rect.bind(cmdBuffer);
        rect.draw(cmdBuffer,rect.indices.size());
        renderer.endRecord(cmdBuffer);
    }

    MyGeo::Vec3f pointRotate(const MyGeo::Vec3f& position, const MyGeo::Vec3f& center, const MyGeo::Vec3f& leftDragVec)
    {
        auto angle=leftDragVec.norm2()/(float)(myswapChain.swapChainExtent.width*myswapChain.swapChainExtent.height);
        auto leftDragVec_normalized=leftDragVec.normalVec();
        auto axis=leftDragVec_normalized.cross(MyGeo::Vec3f{0,0,1});
        auto res=rotationMat(axis,angle)(MyGeo::Vec4f{position-center,1.f}).head+center;
        return res;
    }

    // MyGeo::Vec3f camPos=MyGeo::Vec3f{0,2,-6};
    // MyGeo::Vec3f eyePos=MyGeo::Vec3f{0,2,-6};

    MyGeo::Vec3f camPos=MyGeo::Vec3f{0,3,-15};
    MyGeo::Vec3f eyePos=MyGeo::Vec3f{0,3,-15};

    MyGeo::Vec3f lookat=MyGeo::Vec3f{0,3,0};

    virtual void updateFrameData(uint32_t currentImage) 
    {

        static auto startTime=mytime::now();
        auto currentTime = mytime::now();
        float time_elapsed=mytime::getDuration(startTime,currentTime)*0.1f;

        UniformBufferObject ubo{};
        auto modelMat=MyGeo::Eye<float,4>();
        if(mydevice.mywindow.leftmousePressed && mydevice.mywindow.leftDragVec!=MyGeo::Vec2f{0,0}) 
        {
            MyGeo::Vec3f leftDragVec={mydevice.mywindow.leftDragVec,0};
            camPos=pointRotate(camPos,lookat,leftDragVec);
        }

static MyGeo::Vec3f xaxis,yaxis,zaxis;
        zaxis=(camPos-lookat).normalVec();
        yaxis={0,1,0};
        xaxis=yaxis.cross(zaxis).normalVec();
        yaxis=zaxis.cross(xaxis);

        if(mydevice.mywindow.rightmousePressed && mydevice.mywindow.rightDragVec!=MyGeo::Vec2f{0,0}) 
        {
            MyGeo::Vec3f rightDragVec={mydevice.mywindow.rightDragVec,0};
            camPos+=0.01*(xaxis*(rightDragVec.x/(float)myswapChain.swapChainExtent.width)-yaxis*(rightDragVec.y/(float)myswapChain.swapChainExtent.height));
            lookat+=0.01*(xaxis*(rightDragVec.x/(float)myswapChain.swapChainExtent.width)-yaxis*(rightDragVec.y/(float)myswapChain.swapChainExtent.height));
        }

        MyGeo::Camera cam{camPos,lookat,{0,1,0}};
        cam.setNearFar(-0.1,-50);

        float fov=40-2*mydevice.mywindow.mousescrollVal;
        cam.setFov(fov, myswapChain.swapChainExtent.width / (float)  myswapChain.swapChainExtent.height);
        cam.updateMat();
        ubo.model=modelMat;
        ubo.view=cam.viewMat;
        ubo.proj=cam.projMat;
        ubo.lightColor={8,8,8};
        ubo.lightPos={4,2,4};
        ubo.eyePos=eyePos;
        ubo.specFactor=1;
        void* data;
        vkMapMemory(mydevice.device, myappdata.uniformBuffers[currentImage].memory, 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(mydevice.device, myappdata.uniformBuffers[currentImage].memory);
    }
private:

};

