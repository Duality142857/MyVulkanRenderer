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

class CsTest
{
public:
    int width=800, height=600;

    Dispatcher dispatcher;
    MyWindow mywindow{width,height};
    MyDevice mydevice{mywindow};
    MySwapChain myswapChain{mydevice};
    Descriptors descriptors{mydevice};

    std::vector<VkDescriptorSet> descriptorSets_scene(descriptorSetFrames);


    void run()
    {
        init();
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


    }
    void createDescriptors()
    {
        //!create pool
        std::vector<VkDescriptorPoolSize> poolsizes;
        poolsizes.push_back(descriptors.poolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,100));
        poolsizes.push_back(descriptors.poolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,100));
        descriptors.createDescriptorPool(poolsizes,1000);

        //!create descriptorSetLayout
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.push_back(descriptors.setLayoutBinding(0,1,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT));
        layoutBindings.push_back(descriptors.setLayoutBinding(1,1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT));
        layoutBindings.push_back(descriptors.setLayoutBinding(2,1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT));
        descriptors.createDescriptorSetLayout(descriptorSetLayout,layoutBindings);

        //!allocateDescriptorsets
        std::vector<VkDescriptorSetLayout> setLayouts(maxFramesInFlight,descriptorSetLayout);
        descriptors.allocateDescriptorSets(setLayouts,descriptorSets_scene);
    }

    void cleanup()
    {

    }

};

int main()
{
    CsTest app;
    app.run();
}
