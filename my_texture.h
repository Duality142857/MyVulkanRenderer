#pragma once
#include"my_device.h"
// #define STB_IMAGE_IMPLEMENTATION
// #include<stb_image.h>

class MyTexture
{
public:
    MyDevice& mydevice;

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

    MyTexture(MyDevice& mydevice,const std::string& filename):mydevice{mydevice}
    {
        createTextureImage(filename);
        createTextureImageView();
        createTextureSampler();
    }

    void createTextureImage(const std::string& filename);
    
    void createTextureImageView();

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    
    void createTextureSampler();

};
