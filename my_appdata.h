#pragma once
#include"my_device.h"
#include"my_buffer.h"
#include"my_swapchain.h"
#include<mygeo/vec.h>
#include"geometry/space.h"
#include<mygeo/mat.h>
#include"my_model.h"
#include"my_texture.h"
// #include<mygeo/geo.h>


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

class MyAppData
{
public:
    MySwapChain& myswapChain;
    MyDevice& mydevice;
    MyTexture mytexture{mydevice,"../resources/MC003_Kozakura_Mari.png"};
    // MyTexture mytexture{mydevice,"../resources/tex-models/cat-atlas.jpg"};


    
    // VkImage textureImage;
    // VkDeviceMemory textureImageMemory;
    // VkImageView textureImageView;
    // VkSampler textureSampler;

    // MyBuffer vertexBuffer{mydevice};
    // MyBuffer indexBuffer{mydevice};
    std::vector<MyBuffer> uniformBuffers;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    VkDescriptorSetLayout descriptorSetLayout;


    std::array<MyVertex_Default,4> vertices{
        MyVertex_Default{{-0.5f, -0.5f, 0.f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.f}, {1.0f, 1.0f, 1.0f}}
    };
    std::array<uint32_t,6> indices{
        0, 1, 2, 2, 3, 0
    };

public:
    MyAppData(MySwapChain& myswapChain):myswapChain{myswapChain},mydevice{myswapChain.mydevice}
    {
        prepareData();
        createData();
    }

    virtual void prepareData() 
    {

    }

    virtual void createData()
    {
        std::cout<<"createData..."<<std::endl;
        // createVertexBuffer();
        // createIndexBuffer();
        createUniformBuffers();
        createDescriptorSetLayout();
        createDescriptorPool();
        createDescriptorSets();
    }

    virtual void cleanup()
    {
        // vertexBuffer.clear();
        // indexBuffer.clear();
        for(auto& ub:uniformBuffers) ub.clear();
        vkDestroyDescriptorPool(mydevice.device, descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(mydevice.device, descriptorSetLayout, nullptr);
    }

    
    // virtual void createVertexBuffer() 
    // {
    //     createDataBuffer(vertices.data(),vertices.size(),vertexBuffer,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    // }
    // virtual void createIndexBuffer() 
    // {
    //     createDataBuffer(indices.data(),indices.size(),indexBuffer,VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    // }
    
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



    virtual void createDescriptorPool() 
    {
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(myswapChain.images.size());
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(myswapChain.images.size());

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(myswapChain.images.size());

        if (vkCreateDescriptorPool(mydevice.device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) 
            throw std::runtime_error("failed to create descriptor pool!");
    }

    virtual void createDescriptorSets() {
        std::vector<VkDescriptorSetLayout> layouts(myswapChain.images.size(), descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(myswapChain.images.size());
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(myswapChain.images.size());
        if (vkAllocateDescriptorSets(mydevice.device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) 
            throw std::runtime_error("failed to allocate descriptor sets!");

        for (size_t i = 0; i < myswapChain.images.size(); i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i].buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = mytexture.textureImageView;
            imageInfo.sampler = mytexture.textureSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(mydevice.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    virtual void createDescriptorSetLayout() 
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
        // std::array<VkDescriptorSetLayoutBinding, 1> bindings = {uboLayoutBinding};

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(mydevice.device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) 
            throw std::runtime_error("failed to create descriptor set layout!");
    }

    template<class T>//common method to create vertex/index buffer
    void createDataBuffer(T* vd, size_t num, MyBuffer& mybuffer, VkBufferUsageFlagBits bufferUsageFlagBit)
    {
        VkDeviceSize bufferSize = sizeof(T) * num;
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        mydevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(mydevice.device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vd, (size_t) bufferSize);
        vkUnmapMemory(mydevice.device, stagingBufferMemory);

        mydevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferUsageFlagBit, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mybuffer.buffer, mybuffer.memory);

        mydevice.copyBuffer(stagingBuffer, mybuffer.buffer, bufferSize);

        vkDestroyBuffer(mydevice.device, stagingBuffer, nullptr);
        vkFreeMemory(mydevice.device, stagingBufferMemory, nullptr);
    }

};