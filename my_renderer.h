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
    MyPipeline& mypipeline;
    MyDevice& mydevice;
    MySwapChain& myswapChain;
    Dispatcher& eventDispatcher;
    // MyTexture mytexture{mydevice,"D:/approot/MyVulkan/resources/tex-models/bunny-atlas.jpg"};
    // BoxModel box{mydevice,myswapChain};
    // ExtModel extmodel{mydevice,myswapChain,"../resources/Marry.obj"};
    // ExtModel extmodel{mydevice,myswapChain,"../resources/teapot.obj"};
    // ShapeModel rect{mydevice,myswapChain,GeoShape::Rect,{10.f,10.f}};

    // ExtModel extmodel{mydevice,myswapChain,"../resources/tex-models/cat.obj"};


    


    std::vector<VkCommandBuffer> commandBuffers;
    
    size_t currentFrame = 0;
static constexpr int MAX_FRAMES_IN_FLIGHT=2;

    MyRenderer(Dispatcher& dispatcher, MyPipeline& mypipeline):mypipeline{mypipeline},mydevice{mypipeline.mydevice},myswapChain{mypipeline.myswapChain},eventDispatcher{dispatcher}
    {
        eventDispatcher.subscribe(WindowResized_Event,[this](const Event& event){onEvent(event);});
        init();
    }

    void init()
    {
        allocateCommandBuffers();
        // recordCommands();
    }


    void endFrame(uint32_t imageIndex)
    {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkPipelineStageFlags waitDstStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &myswapChain.imageAvailableSemaphores[currentFrame];
        submitInfo.pWaitDstStageMask = &waitDstStageMask;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &myswapChain.renderFinishedSemaphores[currentFrame];

        //when rendering commands finished, signal semaphore as well as signal frameFence of current frame, the former means it is ready to present, the latter means 
        if (vkQueueSubmit(mydevice.graphicsQueue, 1, &submitInfo, myswapChain.frameFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &myswapChain.renderFinishedSemaphores[currentFrame];

        VkSwapchainKHR swapChains[] = {myswapChain.swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        auto result = vkQueuePresentKHR(mydevice.presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mydevice.mywindow.framebufferResized) 
        {
            // mydevice.mywindow.framebufferResized = false;
            // recreateSwapChain();
            WindowResizedEvent event;
            std::cout<<"dispatch!"<<std::endl;
            eventDispatcher.dispatch(event);
        } 
        else if (result != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }


    void onEvent(const Event& event)
    {
        if(event.type()==WindowResized_Event)
        {
            std::cout<<"dealing with WindowResizedEvent"<<std::endl;
            mydevice.mywindow.framebufferResized = false;
            recreateSwapChain();
        }
    }

    void recreateSwapChain()
    {
        int width=0,height=0;
        glfwGetFramebufferSize(mydevice.mywindow.window,&width,&height);
        while (width==0||height==0)
        {
            glfwGetFramebufferSize(mydevice.mywindow.window, &width, &height);
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(mydevice.device);
        
        clear();
        reboot();
    }



    void reboot()
    {
        myswapChain.init();
        mypipeline.createGraphicsPipeline();
        init();
    }

    void clear()
    {
        myswapChain.clear();
        // myappdata.clear();
        mypipeline.clear();
        // vkFreeCommandBuffers(mydevice.device,mydevice.commandPool,static_cast<uint32_t>(commandBuffers.size()),commandBuffers.data());
    }

    void cleanup()
    {
        myswapChain.cleanup();
        // myappdata.cleanup();
        mypipeline.cleanup();
        vkFreeCommandBuffers(mydevice.device,mydevice.commandPool,static_cast<uint32_t>(commandBuffers.size()),commandBuffers.data());
    }


    virtual void allocateCommandBuffers() 
    {
        // commandBuffers.resize(myswapChain.framebuffers.size());
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);


        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = mydevice.commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();
        std::cout<<"commandbuffercount: "<<allocInfo.commandBufferCount<<std::endl;

        if (vkAllocateCommandBuffers(mydevice.device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) 
            throw std::runtime_error("failed to allocate command buffers!");
    }



    uint32_t startFrame()
    {
        vkWaitForFences(mydevice.device, 1, &myswapChain.frameFences[currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(mydevice.device, 1, &myswapChain.frameFences[currentFrame]);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(mydevice.device, myswapChain.swapChain, UINT64_MAX, myswapChain.imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) 
        {
            recreateSwapChain();
            return -1;
        } 
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
        return imageIndex;
        
    }


    void startRecord(VkCommandBuffer cmdbuffer, VkFramebuffer framebuffer, VkPipeline pipeline) 
    {
        VkCommandBufferBeginInfo cmdbeginInfo{};
        cmdbeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if(vkBeginCommandBuffer(cmdbuffer,&cmdbeginInfo)!=VK_SUCCESS)
        { 
            std::cerr<<" file "<<__FILE__<<" line "<<__LINE__<<std::endl;
            throw std::runtime_error("failed to begin recording!");
        }
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = myswapChain.renderPass;
        renderPassInfo.framebuffer = framebuffer;
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = myswapChain.swapChainExtent;
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();  
        vkCmdBeginRenderPass(cmdbuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    void endRecord(VkCommandBuffer cmdbuffer)
    {
        vkCmdEndRenderPass(cmdbuffer);
        if (vkEndCommandBuffer(cmdbuffer) != VK_SUCCESS)
            throw std::runtime_error("failed to record command buffer!");
    }
};