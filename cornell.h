#pragma once

#include <iostream>
#include"geometry/bbox.h"
#include"geometry/ray.h"
#include"geometry/transforms.h"
#include"geometry/shapes.h"
#include"geometry/instance.h"
#include"geometry/mesh.h"
#include"core/object.h"
#include"implement/mesh.inpl"
#include"my_model.h"
template<class A,class B, class... Args>
std::shared_ptr<A> genShared(const Args&... args)
{
    return std::make_shared<B>(args...);
}

class BoxModel: public MyModel 
{
public:
    std::vector<MyVertex_Default> vertices;
    std::vector<uint32_t> indices;
    BoxModel(MyDevice& mydevice,MySwapChain& myswapChain):MyModel{mydevice,myswapChain}
    {
        createData();
    }

    void createData() override
    {
        Cube cube{{1,1,1}};
        for(auto i=0;i!=cube.positions.size();++i)
        {
            vertices.emplace_back(MyVertex_Default{{cube.positions[i].v3},{1,1,1},{cube.normals[i].v3}});
        }
        for(auto& i:cube.indices)
        {
            indices.emplace_back(i);
        }
        createVertexBuffer();
        createIndexBuffer();
    }

    void draw(VkCommandBuffer commandBuffer, uint32_t size, uint32_t instanceCount) override
    {
        vkCmdDrawIndexed(commandBuffer, size, instanceCount, 0, 0, 0);
    }
    void bind(VkCommandBuffer commandBuffer) override
    {
        VkBuffer vertexBuffers[] = {vertexBuffer.buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    }
    virtual void createVertexBuffer() override
    {
        createDataBuffer(vertices.data(),vertices.size(),vertexBuffer,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }
    virtual void createIndexBuffer() override
    {
        createDataBuffer(indices.data(),indices.size(),indexBuffer,VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    }


};