#pragma once
#include"my_model.h"
#include"my_swapchain.h"
#include"my_descriptors.h"

struct InstanceData
{
    float rotationAngle;
    MyGeo::Vec3f rotationAxis;
    MyGeo::Vec3f scaling;
    MyGeo::Vec3f translation;

    static VkVertexInputBindingDescription getBindingDescription(uint32_t binding)
    {
        VkVertexInputBindingDescription desc{};
        desc.binding=binding;
        desc.stride=sizeof(InstanceData);
        desc.inputRate=VK_VERTEX_INPUT_RATE_INSTANCE;
        return desc;
    }

    static VkVertexInputAttributeDescription attribDesc(uint32_t binding, uint32_t location,VkFormat format,uint32_t offset)
    {
        VkVertexInputAttributeDescription desc{};
        desc.binding=binding;
        desc.location=location;
        desc.format=format;
        desc.offset=offset;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding, uint32_t startLocation) 
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        attributeDescriptions.resize(4);

        attributeDescriptions[0].binding = binding;
        attributeDescriptions[0].location = startLocation;
        attributeDescriptions[0].format = VK_FORMAT_R32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(InstanceData, rotationAngle);

        attributeDescriptions[1].binding = binding;
        attributeDescriptions[1].location = startLocation+1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(InstanceData, rotationAxis);

        attributeDescriptions[2].binding = binding;
        attributeDescriptions[2].location = startLocation+2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(InstanceData, scaling);

        attributeDescriptions[3].binding = binding;
        attributeDescriptions[3].location = startLocation+3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(InstanceData, translation);

        return attributeDescriptions;
    }

};



class MyPipeline
{
public:
    MyDevice& mydevice;
    MySwapChain& myswapChain;
    MyDescriptors& mydescriptors;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

    enum PipelineType{
        Graphics,Offscreen,Compute
    };
    PipelineType pipelineType{Graphics};
    
    std::string vertexShaderFile;
    std::string fragmentShaderFile;
    
    MyPipeline(MyDescriptors& _mydescriptors):mydevice{_mydescriptors.mydevice},myswapChain{_mydescriptors.myswapChain},mydescriptors{_mydescriptors}
    {
        createPipelineLayout();
    }

    virtual void createPipelineLayout()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts =&mydescriptors.descriptorSetLayout;

        if (vkCreatePipelineLayout(mydevice.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) 
            throw std::runtime_error("failed to create pipeline layout!");
    }
  
    //used to recreateswapchain
    void init()
    {
        if(pipelineType==Graphics)
        {
            createGraphicsPipeline(vertexShaderFile,fragmentShaderFile);
        }
    }


    virtual void createdepthPipeline(const std::string& vertexShaderFile, uint32_t pcf=0);

    virtual void createGraphicsPipeline(const std::string& _vertexShaderFile, const std::string& _fragmentShaderFile);
    
    void createInstancePipeline(const std::string& _vertexShaderFile,const std::string& _fragmentShaderFile);
   

    void clear()
    {
        vkDestroyPipeline(mydevice.device,pipeline,nullptr);
    }
    void cleanup()
    {
        clear();
    }
};