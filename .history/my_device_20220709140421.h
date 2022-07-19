#pragma once
// #include<vector>
// #include<optional>
// #include <set>
// #include <algorithm>
// #include<cstring>
// #include<math.h>
// #define GLFW_INCLUDE_VULKAN
// #include<GLFW/glfw3.h>
#include"my_window.h"
#include"my_buffer.h"

// #define NDEBUG
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif


struct QueueFamilyIndices 
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> computeFamily;

    bool isComplete() 
    {
        return graphicsFamily.has_value() && presentFamily.has_value()&& computeFamily.has_value();
    }
};
struct SwapChainSupportDetails 
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

/**
 * creating glfwwindow, instance, physical and logical device
 * ,surface, VkQueues, QueueFamiliyIndices, validationLayers, deviceExtensions
 * containing the corresponding handles
 */ 
class MyDevice
{
public:
    MyDevice(MyWindow& mywindow):mywindow{mywindow}
    {
        init();
    }
    VkDebugUtilsMessengerEXT debugMessenger;
    MyWindow& mywindow;
    VkInstance instance;
    VkDevice device;
    VkPhysicalDevice physicalDevice=VK_NULL_HANDLE;
    VkSurfaceKHR surface;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue computeQueue;
    QueueFamilyIndices queueFamilyIndices;
    VkCommandPool commandPool;

    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    
    void init();
    void cleanup();
    void createInstance();
    
    void createSurface();
    
    void pickPhysicalDevice();

    //get queuefamilyindices from physical device
    //create logical device with device queues(graphics, present, compute)
    void createLogicalDevice();

    void createCommandPool();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    VkCommandBuffer beginSingleTimeCommands();

    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    void createImageWithInfo(VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void createImageViews(std::vector<VkImageView>& imageViews,std::vector<VkImage>& images, VkFormat imageFormat);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    void createDataBuffer(void* vd, VkDeviceSize bufferSize, MyBuffer& mybuffer, VkBufferUsageFlagBits bufferUsageFlagBit);

    void updateDataBuffer(void* vd,VkDeviceSize bufferSize, MyBuffer& mybuffer);

    bool isDeviceSuitable(VkPhysicalDevice device);
    
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    
    bool checkValidationLayerSupport();

    std::vector<const char *> getRequiredExtensions();

    void setupDebugMessenger();

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) 
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) 
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) 
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } 
        else 
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) 
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }
};