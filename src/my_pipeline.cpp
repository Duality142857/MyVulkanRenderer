#include"../my_pipeline.h"
#include"../my_shader.h"
#include"../my_pipelineConfig.h"
void MyPipeline::createGraphicsPipeline(const std::string& _vertexShaderFile, const std::string& _fragmentShaderFile)
{
    vertexShaderFile=_vertexShaderFile;
    fragmentShaderFile=_fragmentShaderFile;
    MyShader vertexShader{mydevice,vertexShaderFile,VK_SHADER_STAGE_VERTEX_BIT};
    MyShader fragmentShader{mydevice,fragmentShaderFile,VK_SHADER_STAGE_FRAGMENT_BIT};
    auto vertShaderStageInfo=vertexShader.getShaderStageCreateInfo();
    auto fragShaderStageInfo=fragmentShader.getShaderStageCreateInfo();
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages{vertShaderStageInfo, fragShaderStageInfo};

    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    bindingDescriptions.resize(1);
    bindingDescriptions[0]=MyVertex_Default::getBindingDescription();

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions=MyVertex_Default::getAttributeDescriptions(0,0);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo=PipelineConfigFunctions::vertexInputStateCreateInfo(bindingDescriptions,attributeDescriptions);
    
    VkPipelineInputAssemblyStateCreateInfo inputAssembly=PipelineConfigFunctions::inputAssemblyStateCreateInfo();
    
    VkPipelineViewportStateCreateInfo viewportStateCI{};
    viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    
    viewportStateCI.viewportCount = 1;
    viewportStateCI.scissorCount = 1;
    
    std::vector<VkDynamicState> dynamicStates={VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo=PipelineConfigFunctions::dynamicStateCreateInfo(dynamicStates);

    VkPipelineRasterizationStateCreateInfo rasterizerStateCI=PipelineConfigFunctions::rasterizerStateCreateInfo(VK_POLYGON_MODE_FILL,VK_CULL_MODE_BACK_BIT,VK_FRONT_FACE_COUNTER_CLOCKWISE);
    
    VkPipelineMultisampleStateCreateInfo multisampling=PipelineConfigFunctions::multisampleCreateInfo();

    VkPipelineDepthStencilStateCreateInfo depthStencil=PipelineConfigFunctions::depthStencilStateCreateInfo();

    VkPipelineColorBlendAttachmentState colorBlendAttachment=PipelineConfigFunctions::colorBlendAttachmentState();

    VkPipelineColorBlendStateCreateInfo colorBlendingStateCI=PipelineConfigFunctions::colorBlendingStateCreateInfo(1,&colorBlendAttachment);

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
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
}
void MyPipeline::createInstancePipeline(const std::string& _vertexShaderFile, const std::string& _fragmentShaderFile)
{
    vertexShaderFile=_vertexShaderFile;
    fragmentShaderFile=_fragmentShaderFile;
    MyShader vertexShader{mydevice,vertexShaderFile,VK_SHADER_STAGE_VERTEX_BIT};
    MyShader fragmentShader{mydevice,fragmentShaderFile,VK_SHADER_STAGE_FRAGMENT_BIT};
    auto vertShaderStageInfo=vertexShader.getShaderStageCreateInfo();
    auto fragShaderStageInfo=fragmentShader.getShaderStageCreateInfo();
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // VkViewport viewport=PipelineConfigFunctions::viewport((float) myswapChain.swapChainExtent.width,(float) myswapChain.swapChainExtent.height);

    // VkRect2D scissor=PipelineConfigFunctions::scissor(myswapChain.swapChainExtent);

    std::vector<VkVertexInputBindingDescription> bindingDescs;
    bindingDescs.resize(2);
    bindingDescs[0]=MyVertex_Default::getBindingDescription(0);
    bindingDescs[1]=InstanceData::getBindingDescription(1);
    
    std::vector<VkVertexInputAttributeDescription> attribDescs=MyVertex_Default::getAttributeDescriptions();
    attribDescs=PipelineConfigFunctions::mergeAttributeDescriptions(attribDescs,InstanceData::getAttributeDescriptions(1,5));

    VkPipelineVertexInputStateCreateInfo vertexInputInfo=PipelineConfigFunctions::vertexInputStateCreateInfo(bindingDescs,attribDescs);
    
    VkPipelineInputAssemblyStateCreateInfo inputAssembly=PipelineConfigFunctions::inputAssemblyStateCreateInfo();
    
    VkPipelineViewportStateCreateInfo viewportStateCI{};
    viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    
    viewportStateCI.viewportCount = 1;
    // viewportState.pViewports = &viewport;
    viewportStateCI.scissorCount = 1;
    // viewportState.pScissors = &scissor;
    
    std::vector<VkDynamicState> dynamicStates={VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo=PipelineConfigFunctions::dynamicStateCreateInfo(dynamicStates);
    
    VkPipelineRasterizationStateCreateInfo rasterizerStateCI=PipelineConfigFunctions::rasterizerStateCreateInfo(VK_POLYGON_MODE_FILL,VK_CULL_MODE_BACK_BIT,VK_FRONT_FACE_COUNTER_CLOCKWISE);
    
    VkPipelineMultisampleStateCreateInfo multisampling=PipelineConfigFunctions::multisampleCreateInfo();

    VkPipelineDepthStencilStateCreateInfo depthStencil=PipelineConfigFunctions::depthStencilStateCreateInfo();

    VkPipelineColorBlendAttachmentState colorBlendAttachment=PipelineConfigFunctions::colorBlendAttachmentState();

    VkPipelineColorBlendStateCreateInfo colorBlendingStateCI=PipelineConfigFunctions::colorBlendingStateCreateInfo(1,&colorBlendAttachment);

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
    pipelineInfo.pDynamicState=&dynamicStateCreateInfo;
    pipelineInfo.renderPass = myswapChain.renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(mydevice.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
        throw std::runtime_error("failed to create graphics pipeline!");
}
void MyPipeline::createdepthPipeline(const std::string& vertexShaderFile, uint32_t pcf)
{
    pipelineType=PipelineType::Offscreen;
    MyShader vertexShader{mydevice,vertexShaderFile,VK_SHADER_STAGE_VERTEX_BIT};
    auto vertShaderStageInfo=vertexShader.getShaderStageCreateInfo();
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {vertShaderStageInfo};

    std::vector<VkVertexInputBindingDescription> bindingDescriptions{MyVertex_Default::getBindingDescription()};

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions=MyVertex_Default::getAttributeDescriptions(0,0);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo=PipelineConfigFunctions::vertexInputStateCreateInfo(bindingDescriptions,attributeDescriptions);
    
    VkPipelineInputAssemblyStateCreateInfo inputAssembly=PipelineConfigFunctions::inputAssemblyStateCreateInfo();
    
    VkPipelineViewportStateCreateInfo viewportStateCI{};
    viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCI.viewportCount = 1;
    viewportStateCI.scissorCount = 1;

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
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo=PipelineConfigFunctions::dynamicStateCreateInfo(dynamicStates);
    
    VkPipelineRasterizationStateCreateInfo rasterizerStateCI=PipelineConfigFunctions::rasterizerStateCreateInfo(VK_POLYGON_MODE_FILL,VK_CULL_MODE_BACK_BIT,VK_FRONT_FACE_COUNTER_CLOCKWISE,VK_TRUE);
    
    VkPipelineMultisampleStateCreateInfo multisampling=PipelineConfigFunctions::multisampleCreateInfo();

    VkPipelineDepthStencilStateCreateInfo depthStencil=PipelineConfigFunctions::depthStencilStateCreateInfo();

    VkPipelineColorBlendAttachmentState colorBlendAttachment=PipelineConfigFunctions::colorBlendAttachmentState();

    VkPipelineColorBlendStateCreateInfo colorBlendingStateCI=PipelineConfigFunctions::colorBlendingStateCreateInfo(0,&colorBlendAttachment);

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
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

}








