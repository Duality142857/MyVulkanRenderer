#pragma
// #include"my_pch.h"

template<class InstanceDataType>
class MyInstances
{
public:
    MyDevice& mydevice;
    MySwapChain& myswapChain;
    MyBuffer instanceBuffer{mydevice};
    std::vector<InstanceDataType> instances;
    
    MyInstances(MyDevice& mydevice,MySwapChain& myswapChain):mydevice{mydevice},myswapChain{myswapChain}
    {}

    void createInstanceBuffer()
    {
        mydevice.createDataBuffer(instances.data(),sizeof(instances[0])*instances.size(),instanceBuffer,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }

    void updateInstanceBuffer()
    {
        mydevice.updateDataBuffer(instances.data(),sizeof(instances[0])*instances.size(),instanceBuffer);
    }

    void bind(VkCommandBuffer cmdBuffer,uint32_t binding)
    {
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmdBuffer,binding,1,&instanceBuffer.buffer,offsets);
    }
};