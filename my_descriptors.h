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
    void init();

    virtual void createDescriptorPool(uint32_t uniformMaxNum, uint32_t imageSamplerMaxNum);

/**
 * @brief Create a Descriptor Set Layout object
 * with uniformNum uniforms and imageSamplerNum samplers, bound sequantially
 * in shadowmap app, this layout will be shared among pipelines, for example the offscreen pipeline will just need one uniform, or some debug pipeline may only need a sampler, just writedescriptor with corresponding bindings
 */
    virtual void createDescriptorSetLayout(uint32_t uniformNum, uint32_t imageSamplerNum);


    virtual void allocateDescriptorSets();

    VkDescriptorBufferInfo getBufferInfo(const MyBuffer& mybuffer, int bufferSize, int offset=0);

    VkDescriptorImageInfo getImageInfo(VkImageLayout imageLayout, VkImageView imageView, VkSampler sampler);

    void cleanup();
};