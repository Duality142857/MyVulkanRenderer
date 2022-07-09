#pragma once

#include"my_buffer.h"
// #define TINYOBJLOADER_IMPLEMENTATION
// #include"ext/tiny_obj_loader.h"
// #include<unordered_map>
#include"my_swapchain.h"
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
    
    static VkVertexInputAttributeDescription attribDesc(uint32_t binding, uint32_t location,VkFormat format,uint32_t offset)
    {
        VkVertexInputAttributeDescription desc{};
        desc.binding=binding;
        desc.location=location;
        desc.format=format;
        desc.offset=offset;
        return desc;
    }
    static VkVertexInputBindingDescription getBindingDescription(uint32_t binding=0) 
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = binding;
        bindingDescription.stride = sizeof(MyVertex_Default);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions(uint32_t binding=0,uint32_t startLocation=0) 
    {
        std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};

        attributeDescriptions[0]=attribDesc(binding,startLocation,VK_FORMAT_R32G32B32_SFLOAT,offsetof(MyVertex_Default, position));

        attributeDescriptions[1]=attribDesc(binding,startLocation+1,VK_FORMAT_R32G32B32_SFLOAT,offsetof(MyVertex_Default, color));

        attributeDescriptions[2]=attribDesc(binding,startLocation+2,VK_FORMAT_R32G32B32_SFLOAT,offsetof(MyVertex_Default, normal));

        attributeDescriptions[3]=attribDesc(binding,startLocation+3,VK_FORMAT_R32G32B32_SFLOAT,offsetof(MyVertex_Default, texCoord));

        attributeDescriptions[4]=attribDesc(binding,startLocation+4,VK_FORMAT_R32_SINT,offsetof(MyVertex_Default, modelId));


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
    virtual void createData() =0;
    void draw(VkCommandBuffer commandBuffer, uint32_t size, uint32_t instanceCount) 
    {
        vkCmdDrawIndexed(commandBuffer, size, instanceCount, 0, 0, 0);
    }
    void bind(VkCommandBuffer commandBuffer) 
    {
        VkBuffer vertexBuffers[] = {vertexBuffer.buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    }
    virtual void createVertexBuffer() 
    {
        mydevice.createDataBuffer(vertices.data(),sizeof(vertices[0])*vertices.size(),vertexBuffer,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }
    virtual void createIndexBuffer() 
    {
        mydevice.createDataBuffer(indices.data(),sizeof(indices[0])*indices.size(),indexBuffer,VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
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
};


class InstanceModel: public MyModel
{
public:
    const MyModel& rawmodel;
    MyGeo::Mat4f modelTransform;
    InstanceModel(const MyModel& _rawmodel ,const MyGeo::Mat4f& mt):MyModel{_rawmodel.mydevice,_rawmodel.myswapChain},rawmodel{_rawmodel},modelTransform{mt}
    {
        vertices.resize(rawmodel.vertices.size());
        memcpy(vertices.data(),rawmodel.vertices.data(), vertices.size()*sizeof(MyVertex_Default));
        for(auto& v:vertices)
        {
            v.position=(modelTransform*MyGeo::Vec4f{v.position,1.f}).head;
            v.normal=(modelTransform*MyGeo::Vec4f{v.normal,1.f}).head;
        }

        indices.resize(rawmodel.indices.size());
        for(auto i=0;i!=indices.size();++i) indices[i]=rawmodel.indices[i];
        createData();
    }
    void createData() override
    {
        createVertexBuffer();
        createIndexBuffer();
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

};