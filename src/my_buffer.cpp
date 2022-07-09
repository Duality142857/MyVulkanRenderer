#include"../my_buffer.h"
#include"../my_device.h"
void MyBuffer::clear()
{
    vkDestroyBuffer(mydevice.device,buffer,nullptr);
    vkFreeMemory(mydevice.device,memory,nullptr);
}