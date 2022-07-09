#include"../my_descriptors.h"

void MyDescriptors::init()
{
    descriptorSets.scene.resize(frameNum);
    descriptorSets.offscreen.resize(frameNum);
    createDescriptorPool(100,100);
    createDescriptorSetLayout(1,2);
    allocateDescriptorSets();
}

void MyDescriptors::createDescriptorPool(uint32_t uniformMaxNum, uint32_t imageSamplerMaxNum) 
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

void MyDescriptors::createDescriptorSetLayout(uint32_t uniformNum, uint32_t imageSamplerNum) 
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


void MyDescriptors::allocateDescriptorSets()
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

VkDescriptorBufferInfo MyDescriptors::getBufferInfo(const MyBuffer& mybuffer, int bufferSize, int offset)
{
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = mybuffer.buffer;
    bufferInfo.offset = offset;
    bufferInfo.range = bufferSize;
    return bufferInfo;
}

VkDescriptorImageInfo MyDescriptors::getImageInfo(VkImageLayout imageLayout, VkImageView imageView, VkSampler sampler)
{
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout=imageLayout;
    imageInfo.imageView=imageView;
    imageInfo.sampler=sampler;
    return imageInfo;
}

void MyDescriptors::cleanup()
{
    vkDestroyDescriptorPool(mydevice.device,pool,nullptr);
    vkDestroyDescriptorSetLayout(mydevice.device,descriptorSetLayout,nullptr);
}


