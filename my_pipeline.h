#pragma once
#include"my_model.h"
#include"my_swapchain.h"
#include"my_descriptors.h"
// #include<fstream>



class MyPipeline
{
public:
    MyDevice& mydevice;
    MySwapChain& myswapChain;
    MyDescriptors& mydescriptors;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

enum PipelineType{
    Graphics,Offscreen
};
    PipelineType pipelineType{Graphics};
    
    std::string vertexShaderFile;
    std::string fragmentShaderFile;
    
    MyPipeline(MyDescriptors& _mydescriptors):mydevice{_mydescriptors.mydevice},myswapChain{_mydescriptors.myswapChain},mydescriptors{_mydescriptors}
    {
        createPipelineLayout();
        // createGraphicsPipeline("../shaders/vert_a.spv","../shaders/frag_a.spv");
    }

    virtual void createPipelineLayout()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts =&mydescriptors.descriptorSetLayout;

        if (vkCreatePipelineLayout(mydevice.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) 
            throw std::runtime_error("failed to create pipeline layout!");
    }
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    
    VkVertexInputBindingDescription bindingDescription{};
    
    std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};
    
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    
    VkPipelineViewportStateCreateInfo viewportStateCI{};
    
    std::vector<VkDynamicState> dynamicStates={VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
    
    VkPipelineRasterizationStateCreateInfo rasterizerStateCI{};
    
    VkPipelineMultisampleStateCreateInfo multisampling{};

    VkPipelineDepthStencilStateCreateInfo depthStencil{};

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};

    VkPipelineColorBlendStateCreateInfo colorBlendingStateCI{};

    VkGraphicsPipelineCreateInfo pipelineInfo{};

    void init()
    {
        if(pipelineType==Graphics)
        {
            createGraphicsPipeline(vertexShaderFile,fragmentShaderFile);
        }
    }

    virtual void presetPipelineCI()
    {

        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        
        bindingDescription = MyVertex_Default::getBindingDescription();
        
        attributeDescriptions = MyVertex_Default::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        

        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        //! not necessary
        // VkViewport viewport{};
        // viewport.x = 0.0f;
        // viewport.y = 0.0f;
        // viewport.width = (float) myswapChain.swapChainExtent.width;
        // viewport.height = (float) myswapChain.swapChainExtent.height;
        // viewport.minDepth = 0.0f;
        // viewport.maxDepth = 1.0f;
        // VkRect2D scissor{};
        // scissor.offset = {0, 0};
        // scissor.extent = myswapChain.swapChainExtent;
    
        viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        
        viewportStateCI.viewportCount = 1;
        // viewportState.pViewports = &viewport;
        viewportStateCI.scissorCount = 1;
        // viewportState.pScissors = &scissor;

        dynamicStateCreateInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.pDynamicStates=dynamicStates.data();
        dynamicStateCreateInfo.dynamicStateCount=dynamicStates.size();
        dynamicStateCreateInfo.flags=0;

        rasterizerStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizerStateCI.depthClampEnable = VK_FALSE;
        rasterizerStateCI.rasterizerDiscardEnable = VK_FALSE;
        rasterizerStateCI.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizerStateCI.lineWidth = 1.0f;
        rasterizerStateCI.cullMode = VK_CULL_MODE_NONE;
        rasterizerStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizerStateCI.depthBiasEnable = VK_FALSE;
        
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        // colorBlendAttachment.colorWriteMask=0xf;
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        colorBlendingStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendingStateCI.logicOpEnable = VK_FALSE;
        colorBlendingStateCI.logicOp = VK_LOGIC_OP_COPY;
        colorBlendingStateCI.attachmentCount = 1;
        colorBlendingStateCI.pAttachments = &colorBlendAttachment;
        colorBlendingStateCI.blendConstants[0] = 0.0f;
        colorBlendingStateCI.blendConstants[1] = 0.0f;
        colorBlendingStateCI.blendConstants[2] = 0.0f;
        colorBlendingStateCI.blendConstants[3] = 0.0f;

        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 1;//only 1 vertex stage
        pipelineInfo.pStages = nullptr;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportStateCI;
        pipelineInfo.pRasterizationState = &rasterizerStateCI;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlendingStateCI;
        pipelineInfo.layout = pipelineLayout;
        //! remember to add dynamicstate later!
        pipelineInfo.pDynamicState=&dynamicStateCreateInfo;
        pipelineInfo.renderPass = myswapChain.renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    }
    
    virtual void createdepthPipeline(const std::string& vertexShaderFile, uint32_t pcf=0)
    {
        pipelineType=PipelineType::Offscreen;
        presetPipelineCI();

        auto vertShaderCode = readFile(vertexShaderFile);
        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo};

        pipelineInfo.stageCount=1;
        pipelineInfo.pStages = shaderStages;

        colorBlendingStateCI.attachmentCount=0;

        rasterizerStateCI.depthBiasEnable=VK_TRUE;
        
        dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
        

        dynamicStateCreateInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.pDynamicStates=dynamicStates.data();
        dynamicStateCreateInfo.dynamicStateCount=dynamicStates.size();
        dynamicStateCreateInfo.flags=0;

        viewportStateCI.pViewports = nullptr;
        viewportStateCI.pScissors = nullptr;

        uint32_t enablePCF=pcf;
        VkSpecializationMapEntry sme{};
        sme.constantID=0;
        sme.offset=0;
        sme.size=sizeof(uint32_t);

        VkSpecializationInfo specializationInfo{};
        specializationInfo.mapEntryCount=1;
        specializationInfo.pMapEntries=&sme;
        specializationInfo.dataSize=sizeof(uint32_t);
        specializationInfo.pData=&enablePCF;

        pipelineInfo.renderPass = myswapChain.offscreenRenderpass;


        if (vkCreateGraphicsPipelines(mydevice.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
            throw std::runtime_error("failed to create graphics pipeline!");

        vkDestroyShaderModule(mydevice.device, vertShaderModule, nullptr);

    }

    virtual void createGraphicsPipelineold(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) 
    {
        auto vertShaderCode = readFile(vertexShaderFile);
        auto fragShaderCode = readFile(fragmentShaderFile);

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription = MyVertex_Default::getBindingDescription();
        auto attributeDescriptions = MyVertex_Default::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;
        

        //透视投影变换相关
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) myswapChain.swapChainExtent.width;
        viewport.height = (float) myswapChain.swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = myswapChain.swapChainExtent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        // rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        //注意逆时针或组成三角形，否则不显示了！
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        // rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        // pipelineLayoutInfo.pSetLayouts =&myappdata.descriptorSetLayout;
        pipelineLayoutInfo.pSetLayouts =&mydescriptors.descriptorSetLayout;


        if (vkCreatePipelineLayout(mydevice.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) 
            throw std::runtime_error("failed to create pipeline layout!");

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = myswapChain.renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(mydevice.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
            throw std::runtime_error("failed to create graphics pipeline!");

        vkDestroyShaderModule(mydevice.device, fragShaderModule, nullptr);
        vkDestroyShaderModule(mydevice.device, vertShaderModule, nullptr);
    }
    
    virtual void createGraphicsPipeline(const std::string& _vertexShaderFile, const std::string& _fragmentShaderFile) 
    {
        // createPipelineLayout();
        vertexShaderFile=_vertexShaderFile;
        fragmentShaderFile=_fragmentShaderFile;
        presetPipelineCI();
        auto vertShaderCode = readFile(vertexShaderFile);
        auto fragShaderCode = readFile(fragmentShaderFile);

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    
        //透视投影变换相关
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) myswapChain.swapChainExtent.width;
        viewport.height = (float) myswapChain.swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = myswapChain.swapChainExtent;

        viewportStateCI.pViewports = &viewport;
        viewportStateCI.pScissors = &scissor;

        rasterizerStateCI.cullMode = VK_CULL_MODE_BACK_BIT;
        pipelineInfo.stageCount=2;
        pipelineInfo.pStages = shaderStages;


        std::cout<<"starting create pipeline"<<std::endl;


        if (vkCreateGraphicsPipelines(mydevice.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
            throw std::runtime_error("failed to create graphics pipeline!");

        vkDestroyShaderModule(mydevice.device, fragShaderModule, nullptr);
        vkDestroyShaderModule(mydevice.device, vertShaderModule, nullptr);
    }
    



    void clear()
    {
        vkDestroyPipeline(mydevice.device,pipeline,nullptr);
        // vkDestroyPipelineLayout(mydevice.device,pipelineLayout,nullptr);
    }
    void cleanup()
    {
        clear();
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
    
    VkShaderModule createShaderModule(const std::vector<char>& code) 
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(mydevice.device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) 
            throw std::runtime_error("failed to create shader module!");
        return shaderModule;
    }

};