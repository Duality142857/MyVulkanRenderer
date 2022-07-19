#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// #include<GLFW/glfw3.h>
// #include<iostream>
#include"my_pch.h"
#include"my_window.h"
#include"my_device.h"
#include"my_swapchain.h"
#include"my_buffer.h"
#include"my_pipeline.h"
#include"my_renderer.h"
#include"my_descriptors.h"
#include"my_texture.h"
#include"my_gui.h"
#include"my_sphere.h"
#include"my_event.h"
#include"my_inputsystem.h"
#include"my_scene.h"
#include"time/myrandom.h"
#include"my_instance.h"
#include"my_shader.h"
#include"my_pipelineConfig.h"
#include"descriptor.h"

static inline void log(const std::string& msg)
{
#ifdef WITHLOG
    std::cout<<msg<<std::endl;
#endif
}

static constexpr int descriptorSetFrames=5;
std::vector<VkDescriptorSet> descriptorSets_compute(descriptorSetFrames);


class CsTest
{
public:
    int width=800, height=600;

    Dispatcher dispatcher;
    MyWindow mywindow{width,height};
    MyDevice mydevice{mywindow};
    MySwapChain myswapChain{mydevice};
    Descriptors descriptors{mydevice};
    MyBuffer storageBuffer{mydevice};

    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline computePipeline;

    VkCommandBuffer cmdbuffer;

    uint32_t elementCount=32;




    void run()
    {
        init();
        createDescriptors();
        updateDescriptorSets();
        cleanup();
    }
    //create pipeline, uniformbuffers, models(with index and vertex buffers created), instances
    void init()
    {
        std::vector<uint32_t> computeInput(32);
        std::vector<uint32_t> computeOutput(32);   
        uint32_t n=0;
        std::generate(computeInput.begin(),computeInput.end(),[&n]{return n++;});
        const VkDeviceSize bufferSize=32*sizeof(uint32_t);

        mydevice.createDataBuffer(computeInput.data(),sizeof(computeInput[0])*computeInput.size(),storageBuffer,VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    }
    void createDescriptors()
    {
        //!create pool
        std::vector<VkDescriptorPoolSize> poolsizes;
        poolsizes.push_back(descriptors.poolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,100));
        descriptors.createDescriptorPool(poolsizes,1000);

        //!create descriptorSetLayout
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.push_back(descriptors.setLayoutBinding(0,1,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,VK_SHADER_STAGE_COMPUTE_BIT));

        descriptors.createDescriptorSetLayout(descriptorSetLayout,layoutBindings);

        //!allocateDescriptorsets
        std::vector<VkDescriptorSetLayout> setLayouts(descriptorSetFrames,descriptorSetLayout);
        descriptors.allocateDescriptorSets(setLayouts,descriptorSets_compute);

        //! create pipeline layout
        VkPipelineLayoutCreateInfo pipelineLayoutCI{};
        pipelineLayoutCI.sType=VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCI.setLayoutCount=1;
        pipelineLayoutCI.pSetLayouts=&descriptorSetLayout;
        vkCreatePipelineLayout(mydevice.device,&pipelineLayoutCI,nullptr,&pipelineLayout);




    }

    void updateDescriptorSets()
    {
        VkDescriptorBufferInfo ssboInfo=descriptors.bufferInfo(storageBuffer,32*sizeof(uint32_t));

        std::array<VkWriteDescriptorSet,1> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets_compute[0];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &ssboInfo;
        vkUpdateDescriptorSets(mydevice.device,descriptorWrites.size(),descriptorWrites.data(),0,nullptr);
    }

    void cleanup()
    {

    }

    void createComputePipeline(const std::string& computeShaderFile)
    {
        MyShader computeShader{mydevice,computeShaderFile,VK_SHADER_STAGE_COMPUTE_BIT};
        auto computeShaderStageInfo=computeShader.getShaderStageCreateInfo();


        VkSpecializationMapEntry sme{};
        sme.constantID=0;
        sme.offset=0;
        sme.size=sizeof(uint32_t);
        VkSpecializationInfo specializationInfo{};
        specializationInfo.mapEntryCount=1;
        specializationInfo.pMapEntries=&sme;
        specializationInfo.dataSize=sizeof(uint32_t);
        specializationInfo.pData=&elementCount;

        computeShaderStageInfo.pSpecializationInfo=&specializationInfo;

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {computeShaderStageInfo};


        //! create compute pipeline
        VkComputePipelineCreateInfo computePipelineCI{};
        computePipelineCI.sType=VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        computePipelineCI.layout=pipelineLayout;
        computePipelineCI.flags=0;
        computePipelineCI.stage=shaderStages[0];
        vkCreateComputePipelines(mydevice.device,nullptr,1,&computePipelineCI,nullptr,&computePipeline);




    }

};

int main()
{
    CsTest app;
    app.run();
}
