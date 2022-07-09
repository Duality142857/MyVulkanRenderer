#include"../my_pipeline.h"

void MyPipeline::createGraphicsPipeline(const std::string& _vertexShaderFile, const std::string& _fragmentShaderFile)
{
    vertexShaderFile=_vertexShaderFile;
    fragmentShaderFile=_fragmentShaderFile;
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

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription=MyVertex_Default::getBindingDescription();

    std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions=MyVertex_Default::getAttributeDescriptions(0,0);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    
    VkPipelineViewportStateCreateInfo viewportStateCI{};
    viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    
    viewportStateCI.viewportCount = 1;
    // viewportState.pViewports = &viewport;
    viewportStateCI.scissorCount = 1;
    // viewportState.pScissors = &scissor;
    
    std::vector<VkDynamicState> dynamicStates={VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
    dynamicStateCreateInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.pDynamicStates=dynamicStates.data();
    dynamicStateCreateInfo.dynamicStateCount=dynamicStates.size();
    dynamicStateCreateInfo.flags=0;
    
    VkPipelineRasterizationStateCreateInfo rasterizerStateCI{};
    rasterizerStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerStateCI.depthClampEnable = VK_FALSE;
    rasterizerStateCI.rasterizerDiscardEnable = VK_FALSE;
    rasterizerStateCI.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerStateCI.lineWidth = 1.0f;
    rasterizerStateCI.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerStateCI.depthBiasEnable = VK_FALSE;
    
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

    VkPipelineColorBlendStateCreateInfo colorBlendingStateCI{};
    colorBlendingStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendingStateCI.logicOpEnable = VK_FALSE;
    colorBlendingStateCI.logicOp = VK_LOGIC_OP_COPY;
    colorBlendingStateCI.attachmentCount = 1;
    colorBlendingStateCI.pAttachments = &colorBlendAttachment;
    colorBlendingStateCI.blendConstants[0] = 0.0f;
    colorBlendingStateCI.blendConstants[1] = 0.0f;
    colorBlendingStateCI.blendConstants[2] = 0.0f;
    colorBlendingStateCI.blendConstants[3] = 0.0f;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
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

    if (vkCreateGraphicsPipelines(mydevice.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
        throw std::runtime_error("failed to create graphics pipeline!");

    vkDestroyShaderModule(mydevice.device, fragShaderModule, nullptr);
    vkDestroyShaderModule(mydevice.device, vertShaderModule, nullptr);

}

void MyPipeline::createInstancePipeline(const std::string& _vertexShaderFile, const std::string& _fragmentShaderFile)
{
    vertexShaderFile=_vertexShaderFile;
    fragmentShaderFile=_fragmentShaderFile;
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

    std::array<VkVertexInputBindingDescription,2> bindingDescs;
    bindingDescs[0]=MyVertex_Default::getBindingDescription(0);
    bindingDescs[1]=InstanceData::getBindingDescription(1);
    
    std::array<VkVertexInputAttributeDescription,9> attribDescs;

    auto attribDescs_0=MyVertex_Default::getAttributeDescriptions();
    attribDescs[0]=attribDescs_0[0];
    attribDescs[1]=attribDescs_0[1];
    attribDescs[2]=attribDescs_0[2];
    attribDescs[3]=attribDescs_0[3];
    attribDescs[4]=attribDescs_0[4];

    auto attribDescs_1=InstanceData::getAttributeDescriptions(1,5);
    attribDescs[5]=attribDescs_1[0];
    attribDescs[6]=attribDescs_1[1];
    attribDescs[7]=attribDescs_1[2];
    attribDescs[8]=attribDescs_1[3];


    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.vertexBindingDescriptionCount = bindingDescs.size();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribDescs.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDescs.data();
    vertexInputInfo.pVertexAttributeDescriptions = attribDescs.data();
    
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    
    VkPipelineViewportStateCreateInfo viewportStateCI{};
    viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    
    viewportStateCI.viewportCount = 1;
    // viewportState.pViewports = &viewport;
    viewportStateCI.scissorCount = 1;
    // viewportState.pScissors = &scissor;
    
    std::vector<VkDynamicState> dynamicStates={VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
    dynamicStateCreateInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.pDynamicStates=dynamicStates.data();
    dynamicStateCreateInfo.dynamicStateCount=dynamicStates.size();
    dynamicStateCreateInfo.flags=0;
    
    VkPipelineRasterizationStateCreateInfo rasterizerStateCI{};
    rasterizerStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerStateCI.depthClampEnable = VK_FALSE;
    rasterizerStateCI.rasterizerDiscardEnable = VK_FALSE;
    rasterizerStateCI.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerStateCI.lineWidth = 1.0f;
    rasterizerStateCI.cullMode = VK_CULL_MODE_NONE;
    rasterizerStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerStateCI.depthBiasEnable = VK_FALSE;
    
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

    VkPipelineColorBlendStateCreateInfo colorBlendingStateCI{};
    colorBlendingStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendingStateCI.logicOpEnable = VK_FALSE;
    colorBlendingStateCI.logicOp = VK_LOGIC_OP_COPY;
    colorBlendingStateCI.attachmentCount = 1;
    colorBlendingStateCI.pAttachments = &colorBlendAttachment;
    colorBlendingStateCI.blendConstants[0] = 0.0f;
    colorBlendingStateCI.blendConstants[1] = 0.0f;
    colorBlendingStateCI.blendConstants[2] = 0.0f;
    colorBlendingStateCI.blendConstants[3] = 0.0f;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
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

    if (vkCreateGraphicsPipelines(mydevice.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
        throw std::runtime_error("failed to create graphics pipeline!");

    vkDestroyShaderModule(mydevice.device, fragShaderModule, nullptr);
    vkDestroyShaderModule(mydevice.device, vertShaderModule, nullptr);

}

void MyPipeline::createdepthPipeline(const std::string& _vertexShaderFile, uint32_t pcf)
{
    pipelineType=PipelineType::Offscreen;

    vertexShaderFile=_vertexShaderFile;

    auto vertShaderCode = readFile(vertexShaderFile);

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo};

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription=MyVertex_Default::getBindingDescription();

    std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions=MyVertex_Default::getAttributeDescriptions(0,0);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    
    VkPipelineViewportStateCreateInfo viewportStateCI{};
    viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    
    viewportStateCI.viewportCount = 1;
    viewportStateCI.pViewports = nullptr;
    viewportStateCI.scissorCount = 1;
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
    
    std::vector<VkDynamicState> dynamicStates={VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_DEPTH_BIAS, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
    dynamicStateCreateInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.pDynamicStates=dynamicStates.data();
    dynamicStateCreateInfo.dynamicStateCount=dynamicStates.size();
    dynamicStateCreateInfo.flags=0;
    
    VkPipelineRasterizationStateCreateInfo rasterizerStateCI{};
    rasterizerStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerStateCI.depthClampEnable = VK_FALSE;
    rasterizerStateCI.rasterizerDiscardEnable = VK_FALSE;
    rasterizerStateCI.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerStateCI.lineWidth = 1.0f;
    rasterizerStateCI.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerStateCI.depthBiasEnable = VK_TRUE;
    
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

    VkPipelineColorBlendStateCreateInfo colorBlendingStateCI{};
    colorBlendingStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendingStateCI.logicOpEnable = VK_FALSE;
    colorBlendingStateCI.logicOp = VK_LOGIC_OP_COPY;
    colorBlendingStateCI.attachmentCount = 0;
    colorBlendingStateCI.pAttachments = &colorBlendAttachment;
    colorBlendingStateCI.blendConstants[0] = 0.0f;
    colorBlendingStateCI.blendConstants[1] = 0.0f;
    colorBlendingStateCI.blendConstants[2] = 0.0f;
    colorBlendingStateCI.blendConstants[3] = 0.0f;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 1;
    pipelineInfo.pStages = shaderStages;
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
    pipelineInfo.renderPass = myswapChain.offscreenRenderpass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(mydevice.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
        throw std::runtime_error("failed to create graphics pipeline!");

    // vkDestroyShaderModule(mydevice.device, fragShaderModule, nullptr);
    vkDestroyShaderModule(mydevice.device, vertShaderModule, nullptr);

}


VkShaderModule MyPipeline::createShaderModule(const std::vector<char>& code) 
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








