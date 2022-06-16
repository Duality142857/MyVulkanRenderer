#pragma once
#include"my_device.h"

class MyBuffer
{
public:
    VkBuffer buffer;
    VkDeviceMemory memory;
    MyDevice& mydevice;

public:
    MyBuffer(MyDevice& mydevice):mydevice{mydevice}{}

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory) 
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(mydevice.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(mydevice.device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex =mydevice.findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(mydevice.device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(mydevice.device, buffer, memory, 0);
    }

    void clear()
    {
        vkDestroyBuffer(mydevice.device,buffer,nullptr);
        vkFreeMemory(mydevice.device,memory,nullptr);
    }
};