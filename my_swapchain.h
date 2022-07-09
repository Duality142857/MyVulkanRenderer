#pragma once

#include"my_window.h"
#include"my_device.h"
#define MYLOG(ostr) ostr<<" from Line: "<<__LINE__<<" file: "<<__FILE__<<std::endl;
// static constexpr std::ostream& myLog(std::ostream& ostr)
// {
//     return ostr<<" from Line: "<<__LINE__<<" file: "<<__FILE__<<std::endl;
// }

/**
 * reference MyDevice, creat swapchain, images, imageviews, framebuffers
 */ 
class MySwapChain
{
public:
static constexpr int MAX_FRAMES_IN_FLIGHT=2;
    MyDevice& mydevice;
    VkSwapchainKHR swapChain;
    VkFormat imageFormat;
    VkExtent2D swapChainExtent;
    VkExtent2D offscreenBufferExtent{2048,2048};

    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;

    std::vector<VkImage> depthImages;
    std::vector<VkDeviceMemory> depthImageMemorys;
    std::vector<VkImageView> depthImageViews;

    VkRenderPass renderPass;
    VkRenderPass offscreenRenderpass;
    std::vector<VkFramebuffer> framebuffers;
    
    VkFramebuffer offscreenFrameBuffer;
    VkImage offscreenImage;
    VkImageView offscreenImageView;
    VkDeviceMemory offscreenImageMemory;
    VkSampler offscreenImageSampler;
    //單線程實際上只各自需要一個semaphore
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;

    std::vector<VkFence> frameFences;
    std::vector<VkFence> imageFences;
    
public:
    MySwapChain(MyDevice& mydevice):mydevice{mydevice}
    {
        init();
    }

    void init();

    void clear();

    void clearSyncObjects();

    void cleanup();

    void createSwapChain();

    virtual void createOffscreenRenderpass();

    virtual void createRenderpass();

    void createFramebuffers();

    void createSyncObjects();

    void createOffscreenDepthBuffer();

    void createDepthResources();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    
    VkFormat findDepthFormat();
};
