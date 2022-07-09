#pragma once

// #include"my_pch.h"

// struct UBO_scene
// {
//     alignas(16) MyGeo::Mat4f model;
//     alignas(16) MyGeo::Mat4f view;
//     alignas(16) MyGeo::Mat4f proj;
//     alignas(16) MyGeo::Mat4f lightMVP;
//     alignas(16) MyGeo::Vec3f lightPos;
//     alignas(16) MyGeo::Vec3f lightColor;
//     alignas(16) MyGeo::Vec3f eyePos;
//     alignas(16) MyGeo::Vec3f ks;
//     alignas(16) MyGeo::Vec3f kd;
// };

struct UBO_modelTransforms
{
    MyGeo::Mat4f model[1000];
};

class ModelInstance
{
public:
    ModelInstance(std::shared_ptr<MyModel> _model, const MyGeo::Mat4f& _transform):model{model},worldTransform{_transform}
    {}
    void draw(VkCommandBuffer commandBuffer, uint32_t size, uint32_t instanceCount) 
    {
        model->draw(commandBuffer,size,instanceCount);
    }
    void bind(VkCommandBuffer commandBuffer) 
    {
        model->bind(commandBuffer);
    }
    

private:
    MyGeo::Mat4f worldTransform;
    std::shared_ptr<MyModel> model;
};

class MyScene
{
    
};