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

constexpr uint32_t elementCount=100;
static constexpr int descriptorSetFrames=5;
std::vector<VkDescriptorSet> descriptorSets_compute(descriptorSetFrames);

    std::vector<uint32_t> computeInput(elementCount);
    std::vector<uint32_t> computeOutput(elementCount);  
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

    void run()
    {
        init();
        createDescriptors();
        updateDescriptorSets();
        createComputePipeline("../shaders/testcompute/test.comp.spv");
        work();
        cleanup();
    }
    //create pipeline, uniformbuffers, models(with index and vertex buffers created), instances
    void init()
    {
 
        uint32_t n=0;
        std::generate(computeInput.begin(),computeInput.end(),[&n]{return n++;});
        const VkDeviceSize bufferSize=elementCount*sizeof(uint32_t);

        VkBufferUsageFlagBits bufferUsage=(VkBufferUsageFlagBits)(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

        mydevice.createDataBuffer(computeInput.data(),sizeof(computeInput[0])*computeInput.size(),storageBuffer,bufferUsage);

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
        VkDescriptorBufferInfo ssboInfo=descriptors.bufferInfo(storageBuffer,elementCount*sizeof(uint32_t));

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

    void work()
    {   
        auto cmdbuffer=mydevice.beginSingleTimeCommands();

        VkBufferMemoryBarrier bufferBarrier{};
        bufferBarrier.sType=VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        bufferBarrier.buffer=storageBuffer.buffer;
        bufferBarrier.size=VK_WHOLE_SIZE;
        bufferBarrier.srcAccessMask=VK_ACCESS_HOST_WRITE_BIT;
        bufferBarrier.dstAccessMask=VK_ACCESS_SHADER_READ_BIT;
        bufferBarrier.srcQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
        bufferBarrier.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;

        vkCmdPipelineBarrier(cmdbuffer,
                            VK_PIPELINE_STAGE_HOST_BIT,
                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                            0,
                            0,
                            nullptr,
                            1,
                            &bufferBarrier,
                            0,
                            nullptr);
        
        vkCmdBindPipeline(cmdbuffer,VK_PIPELINE_BIND_POINT_COMPUTE,computePipeline);
        vkCmdBindDescriptorSets(cmdbuffer,VK_PIPELINE_BIND_POINT_COMPUTE,pipelineLayout,0,1,&descriptorSets_compute[0],0,0);
        vkCmdDispatch(cmdbuffer,elementCount,1,1);

        bufferBarrier.srcAccessMask=VK_ACCESS_SHADER_WRITE_BIT;
        bufferBarrier.dstAccessMask=VK_ACCESS_TRANSFER_READ_BIT;
        
        vkCmdPipelineBarrier(
            cmdbuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0,
            nullptr,
            1,
            &bufferBarrier,
            0,
            nullptr
        );

        mydevice.endSingleTimeCommands(cmdbuffer);

const VkDeviceSize bufferSize=elementCount*sizeof(uint32_t);
        mydevice.copyBuffer2host(storageBuffer.buffer,computeOutput.data(),bufferSize);

        for(auto& x:computeOutput)
        {
            std::cout<<x<<',';
        }
        std::cout<<std::endl;

        
        // const VkDeviceSize bufferSize=32*sizeof(uint32_t);
        // VkBufferCopy copyRegion{};
        // copyRegion.size=bufferSize;
        
        // vkCmdCopyBuffer(cmdbuffer,storageBuffer.buffer,)

        


    }

};

int main()
{
    CsTest app;
    app.run();
}
