#pragma once
#include"my_device.h"
#include"my_buffer.h"
#include"my_swapchain.h"
// #include<vector>
// #include"my_pch.h"



//
struct DescriptorStat
{
    uint32_t uniformNum;
    uint32_t imageSamplerNum;
};

struct DescriptorSets
{
    std::vector<VkDescriptorSet> scene;
    std::vector<VkDescriptorSet> offscreen;
}; 


class MyDescriptors
{


public:
    uint32_t frameNum=5;
    // DescriptorStat stat;
    MySwapChain& myswapChain;
    MyDevice& mydevice;

    VkDescriptorPool pool;
    DescriptorSets descriptorSets;

    VkDescriptorSetLayout descriptorSetLayout;
    
    MyDescriptors(MySwapChain& _myswapchain):myswapChain{_myswapchain},mydevice{_myswapchain.mydevice}
    {
        init();
    }


    /**
     * @brief create pool, setLayout, and maxmum sets, according to externally set stats
     * 1. descriptor pool has 2 poolsizes, one with uniforms and one with samplers, 
     * 2. setlayout is shared among all sets, which means each set has all uniforms and samplers 
     * 3. allocate maxmum num of sets
     * 4. sets need yet to be updated and binded.
     */
    void init()
    {
        // descriptorSets.resize(maxSetNum);
        descriptorSets.scene.resize(frameNum);
        descriptorSets.offscreen.resize(frameNum);

        createDescriptorPool(100,100);
        createDescriptorSetLayout(1,2);
        allocateDescriptorSets();
    }

    virtual void createDescriptorPool(uint32_t uniformMaxNum, uint32_t imageSamplerMaxNum) 
    {
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = uniformMaxNum;
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = imageSamplerMaxNum;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = frameNum*2;

        if (vkCreateDescriptorPool(mydevice.device, &poolInfo, nullptr, &pool) != VK_SUCCESS) 
            throw std::runtime_error("failed to create descriptor pool!");
    }

/**
 * @brief Create a Descriptor Set Layout object
 * with uniformNum uniforms and imageSamplerNum samplers, bound sequantially
 * in shadowmap app, this layout will be shared among pipelines, for example the offscreen pipeline will just need one uniform, or some debug pipeline may only need a sampler, just writedescriptor with corresponding bindings
 */
    virtual void createDescriptorSetLayout(uint32_t uniformNum, uint32_t imageSamplerNum) 
    {
        uint32_t currentBinding=0;
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.reserve(uniformNum+imageSamplerNum);

        for(int i=0;i!=uniformNum;++i)
        {
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = currentBinding++;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            layoutBindings.emplace_back(uboLayoutBinding);
        }

        for(int i=0;i!=imageSamplerNum;++i)
        {
            VkDescriptorSetLayoutBinding samplerLayoutBinding{};
            samplerLayoutBinding.binding = currentBinding++;
            samplerLayoutBinding.descriptorCount = 1;
            samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerLayoutBinding.pImmutableSamplers = nullptr;
            samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            layoutBindings.emplace_back(samplerLayoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
        layoutInfo.pBindings = layoutBindings.data();

        if (vkCreateDescriptorSetLayout(mydevice.device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) 
            throw std::runtime_error("failed to create descriptor set layout!");
    }


    virtual void allocateDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(frameNum,descriptorSetLayout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool=pool;
        allocInfo.descriptorSetCount=frameNum;
        allocInfo.pSetLayouts=layouts.data();

        if(vkAllocateDescriptorSets(mydevice.device,&allocInfo,descriptorSets.scene.data())!=VK_SUCCESS || vkAllocateDescriptorSets(mydevice.device,&allocInfo,descriptorSets.offscreen.data())!=VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
        
        
    }

    // virtual void allocateDescriptorSets_old()
    // {
    //     std::vector<VkDescriptorSetLayout> layouts(maxSetNum,descriptorSetLayout);

    //     VkDescriptorSetAllocateInfo allocInfo{};
    //     allocInfo.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    //     allocInfo.descriptorPool=pool;
    //     allocInfo.descriptorSetCount=maxSetNum;
    //     allocInfo.pSetLayouts=layouts.data();

    //     if(vkAllocateDescriptorSets(mydevice.device,&allocInfo,descriptorSets.data())!=VK_SUCCESS)
    //     {
    //         throw std::runtime_error("failed to allocate descriptor sets!");
    //     }
    // }

    VkDescriptorBufferInfo getBufferInfo(const MyBuffer& mybuffer, int bufferSize, int offset=0)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = mybuffer.buffer;
        bufferInfo.offset = offset;
        bufferInfo.range = bufferSize;
        return bufferInfo;
    }

    VkDescriptorImageInfo getImageInfo(VkImageLayout imageLayout, VkImageView imageView, VkSampler sampler)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout=imageLayout;
        imageInfo.imageView=imageView;
        imageInfo.sampler=sampler;
        return imageInfo;
    }

    void cleanup()
    {
        vkDestroyDescriptorPool(mydevice.device,pool,nullptr);
        vkDestroyDescriptorSetLayout(mydevice.device,descriptorSetLayout,nullptr);
    }
};