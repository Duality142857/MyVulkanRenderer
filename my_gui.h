#pragma once
#include<imgui.h>
#include<backends/imgui_impl_glfw.h>
#include<backends/imgui_impl_vulkan.h>
#include"my_window.h"
class MyGui
{
public:
    VkDescriptorPool descriptorPool;
    ImFont* font1;
    MyWindow& mywindow;
    MyDevice& mydevice;
    MySwapChain& myswapChain;

    float fps;
    ImDrawData* drawData;
    MyGui(MySwapChain& _myswapChain):mywindow{_myswapChain.mydevice.mywindow},mydevice{_myswapChain.mydevice},myswapChain{_myswapChain}
    {}

    /**
     * @brief init imgui, upload font
     * 
     */
    void init()
    {
        ImGui::CreateContext();
        ImGuiIO& io=ImGui::GetIO();
        (void)io;
        ImGui_ImplGlfw_InitForVulkan(mywindow.window, true);
        VkDescriptorPoolSize pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };
        

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = std::size(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;

        vkCreateDescriptorPool(mydevice.device, &pool_info, nullptr, &descriptorPool);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = mydevice.instance;
        init_info.PhysicalDevice = mydevice.physicalDevice;
        init_info.Device = mydevice.device;
        init_info.QueueFamily = mydevice.queueFamilyIndices.graphicsFamily.value();
        init_info.Queue = mydevice.graphicsQueue;
        init_info.PipelineCache = nullptr;
        init_info.DescriptorPool = descriptorPool;
        init_info.Subpass = 0;
        init_info.MinImageCount = 2;
        init_info.ImageCount = 2;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = nullptr;

        ImGui_ImplVulkan_Init(&init_info,myswapChain.renderPass);
        
        //!upload fonts 上传字体
        {
            font1=io.Fonts->AddFontFromFileTTF("/home/number/game/SourceHanSerifCN-VF.ttf",20.0f,NULL,io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
            
            vkResetCommandPool(mydevice.device,mydevice.commandPool,0);

            VkCommandBuffer commandBuffer = mydevice.beginSingleTimeCommands();

            ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
            mydevice.endSingleTimeCommands(commandBuffer);

            vkDeviceWaitIdle(mydevice.device);
            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }
        
    }

    void cleanup()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }



};