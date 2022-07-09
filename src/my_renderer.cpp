#include"../my_renderer.h"

void MyRenderer::endFrame(uint32_t imageIndex)
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags waitDstStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &myswapChain.imageAvailableSemaphores[currentFrame];
    submitInfo.pWaitDstStageMask = &waitDstStageMask;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &myswapChain.renderFinishedSemaphores[currentFrame];

    //when rendering commands finished, signal semaphore as well as signal frameFence of current frame, the former means it is ready to present, the latter means 
    if (vkQueueSubmit(mydevice.graphicsQueue, 1, &submitInfo, myswapChain.frameFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &myswapChain.renderFinishedSemaphores[currentFrame];

    VkSwapchainKHR swapChains[] = {myswapChain.swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    auto result = vkQueuePresentKHR(mydevice.presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mydevice.mywindow.framebufferResized) 
    {
        WindowResizedEvent event;
        std::cout<<"dispatch!"<<std::endl;
        eventDispatcher.dispatch(event);
    } 
    else if (result != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void MyRenderer::onEvent(const Event& event)
{
    if(event.type()==WindowResized_Event)
    {
        std::cout<<"dealing with WindowResizedEvent"<<std::endl;
        mydevice.mywindow.framebufferResized = false;
        recreateSwapChain();
    }
}

void MyRenderer::recreateSwapChain()
{
    int width=0,height=0;
    glfwGetFramebufferSize(mydevice.mywindow.window,&width,&height);
    while (width==0||height==0)
    {
        glfwGetFramebufferSize(mydevice.mywindow.window, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(mydevice.device);
    clear();
    reboot();
}

void MyRenderer::reboot()
{
    myswapChain.init();
    scenePipeline.init();
    init();
}

void MyRenderer::clear()
{
    myswapChain.clear();
    scenePipeline.clear();
}

void MyRenderer::cleanup()
{
    myswapChain.cleanup();
    scenePipeline.clear();
    shadowPipeline.clear();
    vkFreeCommandBuffers(mydevice.device,mydevice.commandPool,static_cast<uint32_t>(commandBuffers.size()),commandBuffers.data());
}


void MyRenderer::allocateCommandBuffers() 
{
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = mydevice.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();
    std::cout<<"commandbuffercount: "<<allocInfo.commandBufferCount<<std::endl;

    if (vkAllocateCommandBuffers(mydevice.device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) 
        throw std::runtime_error("failed to allocate command buffers!");
}



uint32_t MyRenderer::startFrame()
{
    vkWaitForFences(mydevice.device, 1, &myswapChain.frameFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(mydevice.device, 1, &myswapChain.frameFences[currentFrame]);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(mydevice.device, myswapChain.swapChain, UINT64_MAX, myswapChain.imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) 
    {
        recreateSwapChain();
        return -1;
    } 
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    return imageIndex;
    
}

void MyRenderer::beginCommandBuffer(VkCommandBuffer cmdbuffer)
{
    VkCommandBufferBeginInfo cmdbeginInfo{};
    cmdbeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if(vkBeginCommandBuffer(cmdbuffer,&cmdbeginInfo)!=VK_SUCCESS)
    { 
        std::cerr<<" file "<<__FILE__<<" line "<<__LINE__<<std::endl;
        throw std::runtime_error("failed to begin recording!");
    }
}