#pragma once
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
#include<iostream>
#include"my_window.h"
#include"my_device.h"
#include"my_swapchain.h"
#include"my_buffer.h"
#include"my_pipeline.h"
#include"my_renderer.h"
#include"my_descriptors.h"
#include"my_texture.h"
#include"cornell.h"
#include"my_gui.h"

struct UniformBufferObject
{
    alignas(16) MyGeo::Mat4f model;
    alignas(16) MyGeo::Mat4f view;
    alignas(16) MyGeo::Mat4f proj;
    alignas(16) MyGeo::Vec3f lightPos;
    alignas(16) MyGeo::Vec3f lightColor;
    alignas(16) MyGeo::Vec3f eyePos;
    alignas(16) float specFactor;
};



class App
{
public:
    int width=800, height=600;
    MyWindow mywindow{width,height};
    MyDevice mydevice{mywindow};
    MySwapChain myswapChain{mydevice};
    DescriptorStats stats{1,1,4,myswapChain.images.size()};
    MyDescriptors mydescriptors{myswapChain,stats};
    
    MyPipeline mypipeline{mydescriptors};
    MyRenderer renderer{mypipeline};
    MyTexture mytexture{mydevice,"../resources/MC003_Kozakura_Mari.png"};

    MyGui gui{myswapChain};
    std::vector<MyBuffer> uniformBuffers;

    ExtModel extmodel{mydevice,myswapChain,"../resources/teapot.obj"};
    ShapeModel rect{mydevice,myswapChain,GeoShape::Rect,{10.f,10.f}};
    App()
    {
        
    }

    virtual void run()
    {
        init();
        mainLoop();
        cleanup();
    }

    virtual void init()
    {
        gui.init();
        createUniformBuffers();
    }


    virtual void cleanup()
    {
        gui.cleanup();
        renderer.cleanup();
        for(auto& ub:uniformBuffers) ub.clear();
        mydescriptors.cleanup();
    }

//At a high level, rendering a frame in Vulkan consists of a common set of steps:
// Wait for the previous frame to finish
// Acquire an image from the swap chain
// Record a command buffer which draws the scene onto that image
// Submit the recorded command buffer
// Present the swap chain image
    virtual void renderFrame() 
    {
        gui.getGuiDrawData();
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
        updateDescriptorSets(cmdBuffer,imageIndex);
        extmodel.draw(cmdBuffer,extmodel.indices.size());
        rect.bind(cmdBuffer);
        rect.draw(cmdBuffer,rect.indices.size());

        ImGui_ImplVulkan_RenderDrawData(gui.drawData,cmdBuffer);

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
        vkMapMemory(mydevice.device, uniformBuffers[currentImage].memory, 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(mydevice.device, uniformBuffers[currentImage].memory);
    }
private:


    virtual void updateDescriptorSets(VkCommandBuffer cmdBuffer, uint32_t imageIndex)
    {   
        uint32_t setIndex=imageIndex%mydescriptors.stats.maxSetNum;

        VkDescriptorBufferInfo bufferInfo=mydescriptors.getBufferInfo(uniformBuffers[imageIndex],sizeof(UniformBufferObject));
        VkDescriptorImageInfo imageInfo=mydescriptors.getImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,mytexture.textureImageView,mytexture.textureSampler);
        
        std::array<VkWriteDescriptorSet,2> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = mydescriptors.descriptorSets[setIndex];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = mydescriptors.descriptorSets[setIndex];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;
        
        vkUpdateDescriptorSets(mydevice.device,descriptorWrites.size(),descriptorWrites.data(),0,nullptr);

        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mypipeline.pipelineLayout, 0, 1, &mydescriptors.descriptorSets[setIndex], 0, nullptr);


    }

    virtual void createUniformBuffers() 
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        // uniformBuffers.resize(myswapChain.images.size());
        for(int i=0;i!=myswapChain.images.size();++i) uniformBuffers.push_back({mydevice});
        // uniformBuffersMemory.resize(swapChainImages.size());

        for (size_t i = 0; i < myswapChain.images.size(); i++) 
        {
            mydevice.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i].buffer, uniformBuffers[i].memory);
        }
    }



};

