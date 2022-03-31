#include "includes/graphics.h"

ScreenMidPointPipeline::ScreenMidPointPipeline(const Renderer * renderer) : GraphicsPipeline(renderer) { }

bool ScreenMidPointPipeline::createGraphicsPipeline(const VkPushConstantRange & pushConstantRange) {
    if (this->renderer == nullptr || !this->renderer->isReady()) return false;

    this->pushConstantRange = pushConstantRange;
  
    if (!this->createScreenMidPoint()) {
        logError("Failed to create Screen Midpoint Pipeline Texture Sampler");
        return false;        
    }

    if (!this->updateGraphicsPipeline()) return false;
    
    return true;
}

bool ScreenMidPointPipeline::updateGraphicsPipeline() {
    if (this->renderer == nullptr || !this->renderer->isReady()) return false;
        
    this->destroyPipelineObjects();
    
    const std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos = this->getShaderStageCreateInfos();
    if (this->getShaderStageCreateInfos().size() < 2) {
        logError("Screen Midpoint Pipeline is missing required shaders");
        return false;
    }
    
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    if (this->vertexBuffer != nullptr) {
        const VkVertexInputBindingDescription bindingDescription = SimpleVertex::getBindingDescription();
        const std::array<VkVertexInputAttributeDescription, 1> attributeDescriptions = SimpleVertex::getAttributeDescriptions();

        vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
        vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    } else {
        vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
        vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
    }
    
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = this->renderer->getSwapChainExtent().width;
    viewport.height = this->renderer->getSwapChainExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = this->renderer->getSwapChainExtent();

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
    rasterizer.polygonMode = this->renderer->doesShowWireFrame() ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
        
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
    pipelineLayoutInfo.pSetLayouts = &this->descriptorSetLayout;
    pipelineLayoutInfo.setLayoutCount = 0;

    VkResult ret = vkCreatePipelineLayout(this->renderer->getLogicalDevice(), &pipelineLayoutInfo, nullptr, &this->layout);
    if (ret != VK_SUCCESS) {
        logError("Failed to Create Screen Midpoint Pipeline Layout!");
        return false;
    }  
    
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
    pipelineInfo.pStages = shaderStageCreateInfos.data();
    pipelineInfo.pVertexInputState = &vertexInputCreateInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.layout = this->layout;
    pipelineInfo.renderPass = this->renderer->getRenderPass();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    ret = vkCreateGraphicsPipelines(this->renderer->getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->pipeline);
    if (ret != VK_SUCCESS) {
        logError("Failed to Create Screen Midpoint Pipeline!");
        return false;
    }

    return true;
}

void ScreenMidPointPipeline::update() {    
}

bool ScreenMidPointPipeline::createDescriptorPool() {
    return true;
}

bool ScreenMidPointPipeline::createDescriptorSetLayout() {
    return true;
}

bool ScreenMidPointPipeline::createDescriptorSets() {
    return true;
}

bool ScreenMidPointPipeline::createScreenMidPoint() {    
    if (this->renderer == nullptr || !this->renderer->isReady()) return false;
    
    VkDeviceSize bufferSize = CROSSHAIR.size() * sizeof(class SimpleVertex);
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    if (!Helper::createBuffer(this->renderer->getPhysicalDevice(), this->renderer->getLogicalDevice(), bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory)) {
            logError("Failed to get Create Screen Midpoint Pipeline Staging Buffer");
            return false;
    }

    void* data = nullptr;
    vkMapMemory(this->renderer->getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, CROSSHAIR.data(), bufferSize);
    vkUnmapMemory(this->renderer->getLogicalDevice(), stagingBufferMemory);

    if (!Helper::createBuffer(this->renderer->getPhysicalDevice(), this->renderer->getLogicalDevice(), bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->vertexBuffer, this->vertexBufferMemory)) {
        logError("Failed to get Screen Midpoint Pipeline Vertex Buffer");
        return false;
    }

    Helper::copyBuffer(this->renderer->getLogicalDevice(), this->renderer->getCommandPool(), this->renderer->getGraphicsQueue(), stagingBuffer,this->vertexBuffer, bufferSize);

    vkDestroyBuffer(this->renderer->getLogicalDevice(), stagingBuffer, nullptr);
    vkFreeMemory(this->renderer->getLogicalDevice(), stagingBufferMemory, nullptr);
    
    return true;
}

void ScreenMidPointPipeline::draw(const VkCommandBuffer & commandBuffer, const uint16_t commandBufferIndex) {
    if (this->isReady() && this->isEnabled() && this->vertexBuffer != nullptr) {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline);

        VkDeviceSize offsets[] = {0};
        VkBuffer vertexBuffers[] = {this->vertexBuffer};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdDraw(commandBuffer, CROSSHAIR.size(), 1, 0, 0);
    }
}

ScreenMidPointPipeline::~ScreenMidPointPipeline() {
    if (this->renderer == nullptr || !this->renderer->isReady()) return;
}
