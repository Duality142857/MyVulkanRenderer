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

class CsTest
{
public:
    int width=800, height=600;

    Dispatcher dispatcher;
    MyWindow mywindow{width,height};
    MyDevice mydevice{mywindow};
    MySwapChain myswapChain{mydevice};


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
    void cleanup()
    {

    }

};

int main()
{
    CsTest app;
    app.run();
}
