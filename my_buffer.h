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

    void clear()
    {
        vkDestroyBuffer(mydevice.device,buffer,nullptr);
        vkFreeMemory(mydevice.device,memory,nullptr);
    }
};