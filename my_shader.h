#pragma once
#include<vector>
#include<fstream>

//create a shader of certain stage, usually we will need to get the VkPipelineShaderStageCreateInfo to feed to the pipeline
class MyShader
{
public:
    std::string shaderSourceFile;
    MyDevice& mydevice;
    VkShaderStageFlagBits stage;
    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    VkShaderModule shaderModule;

    MyShader(MyDevice& _mydevice,const std::string& _shaderSourceFile,VkShaderStageFlagBits _stage):mydevice{_mydevice},shaderSourceFile{_shaderSourceFile},stage{_stage}
    {
        auto shaderSource=readFile(shaderSourceFile);
        createShaderModule(shaderSource);
        shaderStageInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage=stage;
        shaderStageInfo.module=shaderModule;
        shaderStageInfo.pName="main";
    }

    ~MyShader()
    {
        vkDestroyShaderModule(mydevice.device, shaderModule, nullptr);
    }

    VkPipelineShaderStageCreateInfo& getShaderStageCreateInfo() 
    {
        return shaderStageInfo;
    }

    static std::vector<char> readFile(const std::string& filename) 
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    void createShaderModule(const std::vector<char>& code) 
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if (vkCreateShaderModule(mydevice.device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) 
            throw std::runtime_error("failed to create shader module!");
    }
};