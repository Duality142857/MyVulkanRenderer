#pragma once
#include"my_pipeline.h"
#include<chrono>
#include<mygeo/geo.h>
#include"time/mytime.h"
#include"cornell.h"
#include"my_model.h"
#include"geometry/transforms.h"

#include<imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
static constexpr int FrameRate=30;
static constexpr int FrameInterval=1000/FrameRate;
static float mycolor[4]={0.5,0.5,0.5,1.0};
static float sliderfloat=0.5;

class MyRenderer
{
    
public:
    MyPipeline& mypipeline;
    MyAppData& myappdata;
    MyDevice& mydevice;
    MySwapChain& myswapChain;
    // BoxModel box{mydevice,myswapChain};
    // ExtModel extmodel{mydevice,myswapChain,"../resources/Marry.obj"};
    ExtModel extmodel{mydevice,myswapChain,"../resources/casa/casa.obj"};

    


    std::vector<VkCommandBuffer> commandBuffers;
    
    size_t currentFrame = 0;
static constexpr int MAX_FRAMES_IN_FLIGHT=2;

    MyRenderer(MyPipeline& mypipeline):mypipeline{mypipeline},mydevice{mypipeline.myappdata.mydevice},myappdata{mypipeline.myappdata},myswapChain{mypipeline.myswapChain}
    {
        init();
    }

    void init()
    {
        allocateCommandBuffers();
        recordCommands();
    }

    virtual void renderFrame() 
    {
        vkWaitForFences(mydevice.device, 1, &myswapChain.inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(mydevice.device, myswapChain.swapChain, UINT64_MAX, myswapChain.imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) 
        {
            recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        updateFrameData(imageIndex);
        if (myswapChain.imagesInFlight[imageIndex] != VK_NULL_HANDLE) 
        {
            vkWaitForFences(mydevice.device, 1, &myswapChain.imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        myswapChain.imagesInFlight[imageIndex] = myswapChain.inFlightFences[currentFrame];

        // vkResetFences(mydevice.device, 1, &myswapChain.inFlightFences[currentFrame]);
        // vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        // recordCommands();
        // if (myswapChain.imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        //     vkWaitForFences(mydevice.device, 1, &myswapChain.imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        // }
        // myswapChain.imagesInFlight[imageIndex] = myswapChain.inFlightFences[currentFrame];

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {myswapChain.imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

        VkSemaphore signalSemaphores[] = {myswapChain.renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(mydevice.device, 1, &myswapChain.inFlightFences[currentFrame]);

        if (vkQueueSubmit(mydevice.graphicsQueue, 1, &submitInfo, myswapChain.inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {myswapChain.swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(mydevice.presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mydevice.mywindow.framebufferResized) 
        {
            mydevice.mywindow.framebufferResized = false;
            recreateSwapChain();
        } 
        else if (result != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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


    MyGeo::Vec3f pointRotate(const MyGeo::Vec3f& position, const MyGeo::Vec3f& center, const MyGeo::Vec3f& leftDragVec)
    {
        auto angle=leftDragVec.norm2()/(float)(myswapChain.swapChainExtent.width*myswapChain.swapChainExtent.height);
        auto leftDragVec_normalized=leftDragVec.normalVec();
        auto axis=leftDragVec_normalized.cross(MyGeo::Vec3f{0,0,1});
        auto res=rotationMat(axis,angle)(MyGeo::Vec4f{position-center,1.f}).head+center;
        return res;
        // return translateMat(center)(rotationMat(MyGeo::Vec3f{0,0,1}.cross(leftDragVec_normalized),angle)(translateMat(-center)(position)));
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
        // auto modelMat=MyGeo::rotationMatrix({0,1,0},time_elapsed);//*MyGeo::scaleMatrix({std::abs(std::sinf(time_elapsed*0.001)),std::abs(std::cosf(time_elapsed*0.001)),1});
        // MyGeo::Camera cam{(MyGeo::translateMatrix({std::sinf(time_elapsed*0.01),std::cosf(time_elapsed*0.01),std::sinf(time_elapsed*0.01)})*MyGeo::Vec4f{0,2,-6,1}).head,{0,2,0},{0,1,0}};

        
        // std::cout<<"dragVec: "<<leftDragVec<<std::endl;
        if(mydevice.mywindow.leftmousePressed && mydevice.mywindow.leftDragVec!=MyGeo::Vec2f{0,0}) 
        {
            MyGeo::Vec3f leftDragVec={mydevice.mywindow.leftDragVec,0};
            // std::cout<<"camPos "<<camPos<<std::endl;
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
            
            // std::cout<<"campos: "<<camPos<<std::endl;

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
        ubo.lightPos={4,0,-1};
        ubo.eyePos=eyePos;
        // ubo.specFactor=time_elapsed*0.01f;
        ubo.specFactor=1;


        void* data;
        vkMapMemory(mydevice.device, myappdata.uniformBuffers[currentImage].memory, 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(mydevice.device, myappdata.uniformBuffers[currentImage].memory);
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
        myappdata.cleanup();
        mypipeline.cleanup();
        vkFreeCommandBuffers(mydevice.device,mydevice.commandPool,static_cast<uint32_t>(commandBuffers.size()),commandBuffers.data());
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

    virtual void allocateCommandBuffers() 
    {
        commandBuffers.resize(myswapChain.framebuffers.size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = mydevice.commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();
        std::cout<<"commandbuffercount: "<<allocInfo.commandBufferCount<<std::endl;

        if (vkAllocateCommandBuffers(mydevice.device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) 
            throw std::runtime_error("failed to allocate command buffers!");

    }

    virtual void recordCommands()
    {
        for (size_t i = 0; i < commandBuffers.size(); i++) 
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) 
            {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = myswapChain.renderPass;
            renderPassInfo.framebuffer = myswapChain.framebuffers[i];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = myswapChain.swapChainExtent;

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
            clearValues[1].depthStencil = {1.0f, 0};

            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mypipeline.graphicsPipeline);

            // VkBuffer vertexBuffers[] = {myappdata.vertexBuffer.buffer};
            // VkDeviceSize offsets[] = {0};
            // vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

            // vkCmdBindIndexBuffer(commandBuffers[i], myappdata.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
            extmodel.bind(commandBuffers[i]);

            vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mypipeline.pipelineLayout, 0, 1, &myappdata.descriptorSets[i], 0, nullptr);

            // vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(myappdata.indices.size()), 1, 0, 0, 0);
            extmodel.draw(commandBuffers[i],extmodel.indices.size());

            vkCmdEndRenderPass(commandBuffers[i]);

            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("failed to record command buffer!");
        }
    
    }
    
};