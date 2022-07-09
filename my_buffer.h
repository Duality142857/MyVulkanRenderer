#pragma once
class MyDevice;

class MyBuffer
{
public:
    VkBuffer buffer;
    VkDeviceMemory memory;
    MyDevice& mydevice;

public:
    MyBuffer(MyDevice& mydevice):mydevice{mydevice}{}

    void clear();
};