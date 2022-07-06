#pragma once
// #include<vector>
// #include<array>
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

    // VkSemaphore imageAvailableSemaphore;
    // VkSemaphore renderFinishedSemaphore;

    std::vector<VkFence> frameFences;
    std::vector<VkFence> imageFences;
    
public:
    MySwapChain(MyDevice& mydevice):mydevice{mydevice}
    {
        init();
    }

    void init()
    {
        createSwapChain();
        createRenderpass();
        createOffscreenRenderpass();//
        mydevice.createImageViews(imageViews,images, imageFormat);
        createDepthResources();
        createOffscreenDepthBuffer();//
        createFramebuffers();
        createSyncObjects();
    }

    void clear()
    {
        for(auto i=0;i!=depthImages.size();++i)
        {
            vkDestroyImageView(mydevice.device, depthImageViews[i], nullptr);
            vkDestroyImage(mydevice.device, depthImages[i], nullptr);
            vkFreeMemory(mydevice.device, depthImageMemorys[i], nullptr);
        }

        for (auto framebuffer : framebuffers) 
        {
            vkDestroyFramebuffer(mydevice.device, framebuffer, nullptr);
        }
        vkDestroyRenderPass(mydevice.device, renderPass, nullptr);

        for (auto imageView : imageViews) 
        {
            vkDestroyImageView(mydevice.device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(mydevice.device, swapChain, nullptr);
    }

    void clearSyncObjects()
    {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(mydevice.device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(mydevice.device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(mydevice.device, frameFences[i], nullptr);
        }

    }

    void cleanup()
    {
        clear();
        clearSyncObjects();
    }

    void createSwapChain() 
    {
        SwapChainSupportDetails swapChainSupport =mydevice.querySwapChainSupport(mydevice.physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
        std::cout<<extent.height<<","<<extent.width<<std::endl;

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }
        std::cout<<"imageCount: "<<imageCount<<std::endl;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = mydevice.surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices =mydevice.queueFamilyIndices;// findQueueFamilies(physicalDevice);
        //! 多加了computeFamily,待驗證
        uint32_t queueFamilyIndicesArray[] = {indices.graphicsFamily.value(), indices.presentFamily.value(), indices.computeFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) 
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndicesArray;
        } 
        else 
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }
        //! check what is pretransform and currenttransform
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(mydevice.device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(mydevice.device, swapChain, &imageCount, nullptr);
        images.resize(imageCount);
        vkGetSwapchainImagesKHR(mydevice.device, swapChain, &imageCount, images.data());

        imageFormat = surfaceFormat.format;
        swapChainExtent=extent;
        std::cout<<"swapchain imagecout: "<<imageCount<<std::endl;
    }

    virtual void createOffscreenRenderpass()
    {
        VkAttachmentDescription dsc{};
        dsc.format=VK_FORMAT_D16_UNORM;
        dsc.samples=VK_SAMPLE_COUNT_1_BIT;
        dsc.loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR;
        dsc.storeOp=VK_ATTACHMENT_STORE_OP_STORE;
        dsc.stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        dsc.stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE;
        dsc.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
        dsc.finalLayout=VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 0;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 0;				
		subpass.pDepthStencilAttachment = &depthReference;	

		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;	

		VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType=VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &dsc;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpass;
		renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassCreateInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(mydevice.device, &renderPassCreateInfo, nullptr, &offscreenRenderpass) != VK_SUCCESS) 
            throw std::runtime_error("failed to create render pass!");
    }

    virtual void createRenderpass() 
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = imageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        

        VkSubpassDescription subpass{};
    //pipelineBindPoint is a VkPipelineBindPoint value specifying the pipeline type supported for this subpass
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        // std::array<VkAttachmentDescription, 1> attachments = {colorAttachment};
        std::array<VkAttachmentDescription,2> attachments{colorAttachment,depthAttachment};
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(mydevice.device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) 
            throw std::runtime_error("failed to create render pass!");
    }

    void createFramebuffers() 
    {
        framebuffers.resize(imageViews.size());

        MYLOG(std::cout<<"framebuffercount: "<<framebuffers.size())
        // std::cout<<"framebuffercount: "<<framebuffers.size()<<" from Line: "<<__LINE__<<" file: "<<__FILE__<<std::endl;
        

        for (size_t i = 0; i < imageViews.size(); i++) 
        {
            std::array<VkImageView, 2> attachments = {imageViews[i],depthImageViews[i]};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(mydevice.device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) 
                throw std::runtime_error("failed to create framebuffer!");
            // if (vkCreateFramebuffer(mydevice.device, &framebufferInfo, nullptr, &offscreenFrameBuffer) != VK_SUCCESS) 
            //     throw std::runtime_error("failed to create framebuffer!");
        }
    }

    void createSyncObjects() 
    {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        frameFences.resize(MAX_FRAMES_IN_FLIGHT);
        imageFences.resize(images.size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            if (vkCreateSemaphore(mydevice.device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(mydevice.device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(mydevice.device, &fenceInfo, nullptr, &frameFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
            }

        }
    }

    void createOffscreenDepthBuffer()
    {

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent = {offscreenBufferExtent.width,offscreenBufferExtent.height,1};
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_D16_UNORM;//depthFormat
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        // imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        // imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        // imageInfo.flags = 0;

        mydevice.createImageWithInfo(
            imageInfo,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            offscreenImage,
            offscreenImageMemory);

        
        VkImageViewCreateInfo offscreenDepthImageViewInfo{};
        offscreenDepthImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        offscreenDepthImageViewInfo.image = offscreenImage;
        offscreenDepthImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        offscreenDepthImageViewInfo.format = VK_FORMAT_D16_UNORM;;
        offscreenDepthImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        offscreenDepthImageViewInfo.subresourceRange.baseMipLevel = 0;
        offscreenDepthImageViewInfo.subresourceRange.levelCount = 1;
        offscreenDepthImageViewInfo.subresourceRange.baseArrayLayer = 0;
        offscreenDepthImageViewInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(mydevice.device, &offscreenDepthImageViewInfo, nullptr, &offscreenImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
        }

        VkSamplerCreateInfo samplerCI{};
        samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCI.maxAnisotropy = 1.0f;
        samplerCI.magFilter=VK_FILTER_LINEAR;
        samplerCI.minFilter=VK_FILTER_LINEAR;
        samplerCI.mipmapMode=VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCI.addressModeU=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.addressModeV=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.addressModeW=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

        samplerCI.mipLodBias=0.f;
        samplerCI.maxAnisotropy=1.f;
        samplerCI.minLod=0.f;
        samplerCI.maxLod=1.f;
        samplerCI.borderColor=VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        
        if (vkCreateSampler(mydevice.device, &samplerCI, nullptr, &offscreenImageSampler) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create texture sampler!");
        }

        // createOffscreenRenderpass();

        //create frame buffer for offscreen depth
        VkFramebufferCreateInfo framebufferCI{};
        framebufferCI.sType=VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCI.renderPass=offscreenRenderpass;
        framebufferCI.attachmentCount=1;
        framebufferCI.pAttachments=&offscreenImageView;
        framebufferCI.width=offscreenBufferExtent.width;
        framebufferCI.height=offscreenBufferExtent.height;
        framebufferCI.layers=1;

        std::cout<<"offscreenFramebuffer: "<<offscreenBufferExtent.width<<" "<<offscreenBufferExtent.height<<std::endl;

		vkCreateFramebuffer(mydevice.device, &framebufferCI, nullptr, &offscreenFrameBuffer);


    }

    void createDepthResources() 
    {
        VkFormat depthFormat = findDepthFormat();
        // swapChainDepthFormat = depthFormat;

        depthImages.resize(imageViews.size());
        depthImageMemorys.resize(imageViews.size());
        depthImageViews.resize(imageViews.size());

        for (int i = 0; i < depthImages.size(); i++) {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent = {swapChainExtent.width,swapChainExtent.height,1};
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = depthFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            mydevice.createImageWithInfo(
                imageInfo,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                depthImages[i],
                depthImageMemorys[i]);

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = depthImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = depthFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(mydevice.device, &viewInfo, nullptr, &depthImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
            }
        }

    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) 
    {
        for (const auto &availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) 
    {
        for (const auto &availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            std::cout << "Present mode: Mailbox" << std::endl;
            return availablePresentMode;
        }
        }

        std::cout << "Present mode: V-Sync" << std::endl;
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) 
    {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(mydevice.mywindow.window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) 
    {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(mydevice.physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }
    
    VkFormat findDepthFormat() 
    {
        return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }


};
