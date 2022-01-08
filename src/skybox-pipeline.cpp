#include "includes/graphics.h"

SkyboxPipeline::SkyboxPipeline(const Renderer * renderer) : GraphicsPipeline(renderer) { }

bool SkyboxPipeline::createGraphicsPipeline(const VkPushConstantRange & pushConstantRange) {
    if (this->renderer == nullptr || !this->renderer->isReady()) return false;

    this->pushConstantRange = pushConstantRange;

    if (!this->createTextureSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)) {
        logError("Failed to create Skybox Pipeline Texture Sampler");
        return false;        
    }
    
    if (!this->createSkybox()) {
        logError("Failed to create Skybox Pipeline Texture Sampler");
        return false;        
    }

    if (!this->createDescriptorPool()) {
        logError("Failed to create Skybox Pipeline Descriptor Pool");
        return false;
    }

    if (!this->createDescriptorSetLayout()) {
        logError("Failed to create Skybox Pipeline Descriptor Set Layout");
        return false;
    }

    if (!this->createDescriptorSets()) {
        logError("Failed to create Skybox Pipeline Descriptor Sets");
        return false;
    }

    if (!this->updateGraphicsPipeline()) return false;
    
    return true;
}

bool SkyboxPipeline::updateGraphicsPipeline() {
    if (this->renderer == nullptr || !this->renderer->isReady() || this->descriptorSetLayout == nullptr) return false;
        
    this->destroyPipelineObjects();
    
    const std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos = this->getShaderStageCreateInfos();
    if (this->getShaderStageCreateInfos().size() < 2) {
        logError("Skybox Pipeline is missing required shaders");
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
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
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
    pipelineLayoutInfo.setLayoutCount = 1;

    VkResult ret = vkCreatePipelineLayout(this->renderer->getLogicalDevice(), &pipelineLayoutInfo, nullptr, &this->layout);
    if (ret != VK_SUCCESS) {
        logError("Failed to Create Skybox Pipeline Layout!");
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
        logError("Failed to Create Skybox Pipeline!");
        return false;
    }

    return true;
}

void SkyboxPipeline::update() {
    
}

bool SkyboxPipeline::createDescriptorPool() {
    if (this->renderer == nullptr || !this->renderer->isReady()) return false;

    if (this->descriptorPool != nullptr) {
        vkDestroyDescriptorPool(this->renderer->getLogicalDevice(), this->descriptorPool, nullptr);
        this->descriptorPool = nullptr;
    }

    std::vector<VkDescriptorPoolSize> poolSizes(2);

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = this->renderer->getImageCount();
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = this->renderer->getImageCount();

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = this->renderer->getImageCount();

    VkResult ret = vkCreateDescriptorPool(this->renderer->getLogicalDevice(), &poolInfo, nullptr, &this->descriptorPool);
    if (ret != VK_SUCCESS) {
       logError("Failed to Create Skybox Pipeline Descriptor Pool!");
       return false;
    }
    
    return true;
}

bool SkyboxPipeline::createDescriptorSetLayout() {
    if (this->renderer == nullptr || !this->renderer->isReady()) return false;
    
    if (this->descriptorSetLayout != nullptr) {
        vkDestroyDescriptorSetLayout(this->renderer->getLogicalDevice(), this->descriptorSetLayout, nullptr);
        this->descriptorSetLayout = nullptr;
    }
    
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

    VkDescriptorSetLayoutBinding modelUniformLayoutBinding{};
    modelUniformLayoutBinding.binding = 0;
    modelUniformLayoutBinding.descriptorCount = 1;
    modelUniformLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    modelUniformLayoutBinding.pImmutableSamplers = nullptr;
    modelUniformLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layoutBindings.push_back(modelUniformLayoutBinding);

    VkDescriptorSetLayoutBinding samplersLayoutBinding{};
    samplersLayoutBinding.binding = 1;
    samplersLayoutBinding.descriptorCount = 1;
    samplersLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplersLayoutBinding.pImmutableSamplers = nullptr;
    samplersLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layoutBindings.push_back(samplersLayoutBinding);

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = layoutBindings.size();
    layoutInfo.pBindings = layoutBindings.data();

    VkResult ret = vkCreateDescriptorSetLayout(this->renderer->getLogicalDevice(), &layoutInfo, nullptr, &this->descriptorSetLayout);
    if (ret != VK_SUCCESS) {
        logError("Failed to Create Skybox Pipeline Descriptor Set Layout!");
        return false;
    }
    
    return true;
}

bool SkyboxPipeline::createDescriptorSets() {
    if (this->renderer == nullptr || !this->renderer->isReady()) return false;
    
    std::vector<VkDescriptorSetLayout> layouts(this->renderer->getImageCount(), this->descriptorSetLayout);
    
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = this->descriptorPool;
    allocInfo.descriptorSetCount = this->renderer->getImageCount();
    allocInfo.pSetLayouts = layouts.data();

    this->descriptorSets.resize(this->renderer->getImageCount());
    VkResult ret = vkAllocateDescriptorSets(this->renderer->getLogicalDevice(), &allocInfo, this->descriptorSets.data());
    if (ret != VK_SUCCESS) {
        logError("Failed to Allocate Skybox Pipeline Descriptor Sets!");
        return false;
    }

    VkDescriptorImageInfo descriptorImageInfo;
    descriptorImageInfo.sampler = this->textureSampler;
    descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descriptorImageInfo.imageView = this->imageView;

    for (size_t i = 0; i < this->descriptorSets.size(); i++) {
        VkDescriptorBufferInfo uniformBufferInfo{};
        uniformBufferInfo.buffer = this->renderer->getUniformBuffer(i);
        uniformBufferInfo.offset = 0;
        uniformBufferInfo.range = sizeof(struct ModelUniforms);

        std::vector<VkWriteDescriptorSet> descriptorWrites;

        VkWriteDescriptorSet uniformDescriptorSet = {};
        uniformDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uniformDescriptorSet.dstSet = this->descriptorSets[i];
        uniformDescriptorSet.dstBinding = 0;
        uniformDescriptorSet.dstArrayElement = 0;
        uniformDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformDescriptorSet.descriptorCount = 1;
        uniformDescriptorSet.pBufferInfo = &uniformBufferInfo;
        descriptorWrites.push_back(uniformDescriptorSet);
        
        VkWriteDescriptorSet samplerDescriptorSet = {};
        samplerDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        samplerDescriptorSet.dstBinding = 1;
        samplerDescriptorSet.dstArrayElement = 0;
        samplerDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerDescriptorSet.descriptorCount = 1;
        samplerDescriptorSet.pImageInfo = &descriptorImageInfo;
        samplerDescriptorSet.dstSet = this->descriptorSets[i];
        descriptorWrites.push_back(samplerDescriptorSet);

        vkUpdateDescriptorSets(this->renderer->getLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
    
    return true;
}

bool SkyboxPipeline::createSkybox() {    
    if (this->renderer == nullptr || !this->renderer->isReady()) return false;
    
    std::vector<std::unique_ptr<Texture>> skyboxCubeTextures;

    for (auto & s : this->skyboxCubeImageLocations) {
        std::unique_ptr<Texture> texture = std::make_unique<Texture>();
        texture->setPath(Engine::getAppPath(SKYBOX) / s);
        texture->load();
        if (texture->isValid()) {
            skyboxCubeTextures.push_back(std::move(texture));
        }            
    }
    
    if (skyboxCubeTextures.size() != 6) return false;

    VkDeviceSize bufferSize = SKYBOX_VERTICES.size() * sizeof(class SimpleVertex);
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    if (!Helper::createBuffer(this->renderer->getPhysicalDevice(), this->renderer->getLogicalDevice(), bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory)) {
            logError("Failed to get Create Skybox Pipeline Staging Buffer");
            return false;
    }

    void* data = nullptr;
    vkMapMemory(this->renderer->getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, SKYBOX_VERTICES.data(), bufferSize);
    vkUnmapMemory(this->renderer->getLogicalDevice(), stagingBufferMemory);

    if (!Helper::createBuffer(this->renderer->getPhysicalDevice(), this->renderer->getLogicalDevice(), bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->vertexBuffer, this->vertexBufferMemory)) {
        logError("Failed to get Skybox Pipeline Vertex Buffer");
        return false;
    }

    Helper::copyBuffer(this->renderer->getLogicalDevice(), this->renderer->getCommandPool(), this->renderer->getGraphicsQueue(), stagingBuffer,this->vertexBuffer, bufferSize);

    vkDestroyBuffer(this->renderer->getLogicalDevice(), stagingBuffer, nullptr);
    vkFreeMemory(this->renderer->getLogicalDevice(), stagingBufferMemory, nullptr);
    
    stagingBuffer = nullptr;
    stagingBufferMemory = nullptr;
    VkDeviceSize skyboxCubeSize = skyboxCubeTextures.size() * skyboxCubeTextures[0]->getSize();
    
    if (!Helper::createBuffer(this->renderer->getPhysicalDevice(), this->renderer->getLogicalDevice(), skyboxCubeSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory)) {
        logError("Failed to get Create Skybox Pipeline Vertex Buffer");
        return false;
    }

    data = nullptr;
    VkDeviceSize offset = 0;
    vkMapMemory(this->renderer->getLogicalDevice(), stagingBufferMemory, 0, skyboxCubeSize, 0, &data);
    for (auto & tex : skyboxCubeTextures) {
        memcpy(static_cast<char *>(data) + offset, tex->getPixels(), tex->getSize());
        offset += tex->getSize();
    }
    vkUnmapMemory(this->renderer->getLogicalDevice(), stagingBufferMemory);

    if (!Helper::createImage(this->renderer->getPhysicalDevice(), this->renderer->getLogicalDevice(),
        skyboxCubeTextures[0]->getWidth(), skyboxCubeTextures[0]->getHeight(), 
        skyboxCubeTextures[0]->getImageFormat(), 
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        this->cubeImage, this->cubeImageMemory, skyboxCubeTextures.size())) {
            logError("Failed to Create Skybox Images");
            return false;
    }

    Helper::transitionImageLayout(this->renderer->getLogicalDevice(), this->renderer->getCommandPool(), this->renderer->getGraphicsQueue(), this->cubeImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, skyboxCubeTextures.size());
    Helper::copyBufferToImage(this->renderer->getLogicalDevice(), this->renderer->getCommandPool(), this->renderer->getGraphicsQueue(),
        stagingBuffer, this->cubeImage, skyboxCubeTextures[0]->getWidth(), skyboxCubeTextures[0]->getHeight(), skyboxCubeTextures.size());
    Helper::transitionImageLayout(this->renderer->getLogicalDevice(), this->renderer->getCommandPool(), this->renderer->getGraphicsQueue(), this->cubeImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, skyboxCubeTextures.size());
    
    vkDestroyBuffer(this->renderer->getLogicalDevice(), stagingBuffer, nullptr);
    vkFreeMemory(this->renderer->getLogicalDevice(), stagingBufferMemory, nullptr);
    
    this->imageView = 
        Helper::createImageView(this->renderer->getLogicalDevice(), this->cubeImage, skyboxCubeTextures[0]->getImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT, skyboxCubeTextures.size());
    if (this->imageView == nullptr) {
        logError("Failed to Create Skybox Image View!");
        return false;
    }

    return true;
}

void SkyboxPipeline::draw(VkCommandBuffer & commandBuffer, const uint16_t commandBufferIndex) {
    if (this->isReady() && this->isEnabled() && this->vertexBuffer != nullptr) {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->layout, 0, 1, &this->descriptorSets[commandBufferIndex], 0, nullptr);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline);

        VkDeviceSize offsets[] = {0};
        VkBuffer vertexBuffers[] = {this->vertexBuffer};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        
        vkCmdDraw(commandBuffer, SKYBOX_VERTICES.size(), 1, 0, 0);
    }
}

SkyboxPipeline::~SkyboxPipeline() {
    if (this->renderer == nullptr || !this->renderer->isReady()) return;
    
     if (this->cubeImage != nullptr) {
        vkDestroyImage(this->renderer->getLogicalDevice(), this->cubeImage, nullptr);
        this->cubeImage = nullptr;
    }

    if (this->cubeImageMemory != nullptr) {
        vkFreeMemory(this->renderer->getLogicalDevice(), this->cubeImageMemory, nullptr);
        this->cubeImageMemory = nullptr;
    }

    if (this->imageView != nullptr) {
        vkDestroyImageView(this->renderer->getLogicalDevice(), this->imageView, nullptr);
        this->imageView = nullptr;
    }   
}
