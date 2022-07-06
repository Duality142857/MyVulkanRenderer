#pragma once

#include"my_buffer.h"
// #define TINYOBJLOADER_IMPLEMENTATION
// #include"ext/tiny_obj_loader.h"
// #include<unordered_map>
#include"my_sphere.h"

static const MyGeo::Vec2f noTextureUV{-1,-1};

struct MyVertex_Default
{
    MyGeo::Vec3f position;
    MyGeo::Vec3f color;
    MyGeo::Vec3f normal;
    MyGeo::Vec2f texCoord;
    int modelId=1;

    bool operator==(const MyVertex_Default& other) const
    {
        return position==other.position;
    }

    static VkVertexInputBindingDescription getBindingDescription() 
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(MyVertex_Default);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions() 
    {
        std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(MyVertex_Default, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(MyVertex_Default, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(MyVertex_Default, normal);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(MyVertex_Default, texCoord);

        attributeDescriptions[4].binding = 0;
        attributeDescriptions[4].location = 4;
        attributeDescriptions[4].format = VK_FORMAT_R32_SINT;
        attributeDescriptions[4].offset = offsetof(MyVertex_Default, modelId);

        // attributeDescriptions[2].binding = 0;
        // attributeDescriptions[2].location = 2;
        // attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        // attributeDescriptions[2].offset = offsetof(MyVertex, texCoord);

        return attributeDescriptions;
    }

};
inline void hash_combine(std::size_t& seed) { }
template <typename T, typename... Rest>
inline void hash_combine(std::size_t& seed, const T& v, Rest... rest) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    hash_combine(seed, rest...);
}

namespace std 
{
    template<> 
    struct hash<MyVertex_Default> 
    {
        size_t operator()(MyVertex_Default const& v) const 
        {
            size_t seed=3;
            hash_combine(seed,v.position.x,v.position.y,v.position.z);
            return seed;
        }
    };
}


class MyModel
{
public:
    MyDevice& mydevice;
    MySwapChain& myswapChain;
    MyBuffer vertexBuffer{mydevice};
    MyBuffer indexBuffer{mydevice};
    std::vector<MyVertex_Default> vertices;
    std::vector<uint32_t> indices;
    MyGeo::Vec3f ks,kd;

    MyModel(MyDevice& mydevice,MySwapChain& myswapChain):mydevice{mydevice},myswapChain{myswapChain}
    {

    }
    virtual void draw(VkCommandBuffer commandBuffer, uint32_t size, uint32_t instanceCount) =0;
    virtual void bind(VkCommandBuffer commandBuffer) =0;
    virtual void createData() =0;

    virtual void createVertexBuffer()=0;
    virtual void createIndexBuffer()=0;

    template<class T>//common method to create vertex/index buffer
    void createDataBuffer(T* vd, size_t num, MyBuffer& mybuffer, VkBufferUsageFlagBits bufferUsageFlagBit)
    {
        VkDeviceSize bufferSize = sizeof(T) * num;
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(mydevice.device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vd, (size_t) bufferSize);
        vkUnmapMemory(mydevice.device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferUsageFlagBit, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mybuffer.buffer, mybuffer.memory);

        mydevice.copyBuffer(stagingBuffer, mybuffer.buffer, bufferSize);

        vkDestroyBuffer(mydevice.device, stagingBuffer, nullptr);
        vkFreeMemory(mydevice.device, stagingBufferMemory, nullptr);
    }

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) 
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(mydevice.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
            throw std::runtime_error("failed to create buffer!");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(mydevice.device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = mydevice.findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(mydevice.device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate buffer memory!");

        vkBindBufferMemory(mydevice.device, buffer, bufferMemory, 0);
    }

};


enum class GeoShape {
    Box, Circle, Sphere, Rect
};

class ShapeModel: public MyModel
{   
public: 

    GeoShape shape;
    ShapeModel(int modelId, MyDevice& mydevice, MySwapChain& myswapChain, GeoShape shape, const std::initializer_list<float>& scales): MyModel{mydevice,myswapChain},shape{shape}
    {
        switch (shape)
        {
        case GeoShape::Rect :
        {
            vertices.resize(4);
            indices.resize(6);
            auto p=scales.begin();
            float a=*p*0.5f;
            float b=*(p+1)*0.5f;
            vertices={
                {
                    {-a,0,b},
                    {1,1,1},
                    {0,1,0},
                    noTextureUV,modelId
                },
                {
                    {a,0,b},
                    {1,1,1},
                    {0,1,0},
                    noTextureUV,modelId
                },
                {
                    {a,0,-b},
                    {1,1,1},
                    {0,1,0},
                    noTextureUV,modelId
                },
                {
                    {-a,0,-b},
                    {1,1,1},
                    {0,1,0},
                    noTextureUV,modelId
                }
                };
            indices={0,1,2,
                    0,2,3};
            break;
        }

        case GeoShape::Sphere :
        {
            SimpleMesh mesh[2];
            Icosahedron(mesh[0],*scales.begin());
            static int n=4;
            SimpleMesh& mesh0=mesh[0],&mesh1=mesh[1];
            for(int i=0;i!=n;++i)
            {
                SubdivideMesh(mesh0,mesh1);
                std::swap(mesh0,mesh1);
            }

            vertices.resize(mesh1.vertices.size());
            indices.resize(mesh1.indices.size());
            for(auto i=0;i!=vertices.size();i++)
            {
                auto& meshv=mesh1.vertices[i];
                vertices[i].position=meshv.position.v3;
                vertices[i].color=meshv.color;
                vertices[i].normal=meshv.normal.v3;
                // std::cout<<vertices[i].position<<" "<<vertices[i].color<<" "<<vertices[i].normal<<std::endl;
                vertices[i].texCoord=noTextureUV;
                vertices[i].modelId=modelId;
            }
            for(auto i=0;i!=mesh1.indices.size();++i)
            {
                indices[i]=mesh1.indices[i];
            }
            std::cout<<"sphere generated!"<<std::endl;
            break;
        }
        
        default:
            break;
        }

        createData();

    }

    void createData() override
    {
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
        std::cout<<"vert num: "<<vertices.size()<<std::endl;
        createDataBuffer(vertices.data(),vertices.size(),vertexBuffer,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }
    virtual void createIndexBuffer() override
    {
        std::cout<<"ind num: "<<indices.size()<<std::endl;

        createDataBuffer(indices.data(),indices.size(),indexBuffer,VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    }

};



class ExtModel:public MyModel 
{
public:

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

    
    const std::string& filename;
    ExtModel(int modelId,MyDevice& mydevice, MySwapChain& myswapChain, const std::string& filename):MyModel{mydevice,myswapChain},filename{filename}
    {
        createData();
    }
    virtual void loadOBJ()
    {
        vertices.clear();
        indices.clear();

        tinyobj::ObjReaderConfig reader_config;
        reader_config.mtl_search_path="../resources";
        tinyobj::ObjReader reader;
        if (!reader.ParseFromFile(filename, reader_config)) 
        {
            if (!reader.Error().empty()) 
                std::cerr << "TinyObjReader: " << reader.Error();
            exit(1);
        }
        if (!reader.Warning().empty()) 
        std::cout << "TinyObjReader: " << reader.Warning();
        auto& attrib = reader.GetAttrib();
        auto& shapes = reader.GetShapes();
        auto& materials = reader.GetMaterials();  
        for(auto& mat : materials)
        {
            // if(mat.name=="材质")
            {
                ks={mat.specular[0],mat.specular[1],mat.specular[2]};
                kd={mat.diffuse[0],mat.diffuse[1],mat.diffuse[2]};
                std::cout<<"ks: "<<ks<<std::endl;
                std::cout<<"kd: "<<kd<<std::endl;

                break;
            }
        }

        std::cout<<"num of shapes: "<<shapes.size()<<std::endl;
        std::unordered_map<MyVertex_Default, uint32_t> uniqueVertices{};
        for (const auto &shape : shapes) 
        {
            for (const auto &index : shape.mesh.indices) 
            {
                MyVertex_Default vertex{};
                vertex.modelId=1;
                if (index.vertex_index >= 0) 
                {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };

                    vertex.color = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2],
                    };

                }

                if (index.texcoord_index >= 0) {

                    vertex.texCoord={
                        attrib.texcoords[2*size_t(index.texcoord_index)+0],
                        1-attrib.texcoords[2*size_t(index.texcoord_index)+1]
                    };

                }
                

                if (index.normal_index >= 0) 
                {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    };
                }

                // if (index.texcoord_index >= 0) {
                //     vertex.uv = {
                //         attrib.texcoords[2 * index.texcoord_index + 0],
                //         attrib.texcoords[2 * index.texcoord_index + 1],
                //     };
                // }

                if (uniqueVertices.count(vertex) == 0) 
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }

    }

    void createData() override
    {
        loadOBJ();
        createVertexBuffer();
        createIndexBuffer();
    }

    // void createTextureImage(const std::string& texturePath) 
    // {
    //     int texWidth, texHeight, texChannels;
    //     stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    //     VkDeviceSize imageSize = texWidth * texHeight * 4;
    //     if (!pixels) {
    //         throw std::runtime_error("failed to load texture image!");
    //     }
    //     VkBuffer stagingBuffer;
    //     VkDeviceMemory stagingBufferMemory;
    //     createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    //     void* data;
    //     vkMapMemory(mydevice.device, stagingBufferMemory, 0, imageSize, 0, &data);
    //         memcpy(data, pixels, static_cast<size_t>(imageSize));
    //     vkUnmapMemory(mydevice.device, stagingBufferMemory);
    //     stbi_image_free(pixels);
    //     VkImageCreateInfo imageInfo{};
    //     imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    //     imageInfo.imageType = VK_IMAGE_TYPE_2D;
    //     imageInfo.extent.width = texWidth;
    //     imageInfo.extent.height = texHeight;
    //     imageInfo.extent.depth = 1;
    //     imageInfo.mipLevels = 1;
    //     imageInfo.arrayLayers = 1;
    //     imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    //     imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    //     imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //     imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    //     imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    //     imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    //     myswapChain.createImageWithInfo(imageInfo,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
    //     transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    //         copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    //     transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    //     vkDestroyBuffer(mydevice.device, stagingBuffer, nullptr);
    //     vkFreeMemory(mydevice.device, stagingBufferMemory, nullptr);
    // }

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