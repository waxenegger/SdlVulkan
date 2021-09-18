#include "includes/graphics.h"

GraphicsPipeline::GraphicsPipeline(const VkDevice & device) : device(device) {    
    
}

void GraphicsPipeline::addShader(const std::string & filename, const VkShaderStageFlagBits & shaderType) {
    const std::map<std::string, const Shader *>::iterator existingShader = this->shaders.find(filename);
    if (existingShader != this->shaders.end()) {
        logInfo("Shader " + filename + " already exists!");
        return;
    }
    
    std::unique_ptr<Shader> newShader = std::make_unique<Shader>(this->device, filename, shaderType);
    if (newShader->isValid()) this->shaders[filename] = newShader.release();
}

bool GraphicsPipeline::createGraphicsPipeline(
    const size_t size, const VkPhysicalDevice & physicalDevice, const VkRenderPass & renderPass, const VkCommandPool & commandpool, const VkQueue & graphicsQueue,
    const VkExtent2D & swapChainExtent, const VkPushConstantRange & pushConstantRange, bool showWireFrame) {
    
    
    if (!this->createTextureSampler(physicalDevice, this->textureSampler, VK_SAMPLER_ADDRESS_MODE_REPEAT)) {
        logError("Failed to create Pipeline Texture Sampler");
        return false;        
    }
    
    if (!this->createDescriptorPool(size)) {
        logError("Failed to create Pipeline Descriptor Pool");
        return false;
    }

    if (!this->createDescriptorSets(size)) {
        logError("Failed to create Pipeline Descriptor Sets");
        return false;
    }
    
    if (!this->createDescriptorSetLayout()) {
        logError("Failed to create Pipeline Descriptor Set Layout");
        return false;
    }

    const BufferSummary bufferSizes = Models::INSTANCE()->getModelsBufferSizes(true);
    if (!this->createSsboBufferFromModel(physicalDevice, commandpool, graphicsQueue, bufferSizes.ssboBufferSize)) {
        logError("Failed to create SSBO from Models");
        return false;        
    }
    
    if (!this->updateGraphicsPipeline(renderPass, swapChainExtent, pushConstantRange, showWireFrame)) return false;
    
    return true;
}

bool GraphicsPipeline::updateGraphicsPipeline(const VkRenderPass & renderPass, const VkExtent2D & swapChainExtent, const VkPushConstantRange & pushConstantRange, bool showWireFrame) {
    if (this->device == nullptr || this->descriptorSetLayout == nullptr) return false;
        
    this->destroyPipelineObjects();
    
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    const VkVertexInputBindingDescription bindingDescription = ModelVertex::getBindingDescription();
    const std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions = ModelVertex::getAttributeDescriptions();

    vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    
    
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

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
    rasterizer.polygonMode = showWireFrame ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = 
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.blendEnable = VK_TRUE;

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
    pipelineLayoutInfo.setLayoutCount = 1;
    
    if (pushConstantRange.size > 0) {
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    }
    
    VkResult ret = vkCreatePipelineLayout(this->device, &pipelineLayoutInfo, nullptr, &this->layout);
    if (ret != VK_SUCCESS) {
        logError("Failed to Create Pipeline Layout!");
        return false;
    }

    const std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos = this->getShaderStageCreateInfos();
    
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
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    ret = vkCreateGraphicsPipelines(this->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->pipeline);
    if (ret != VK_SUCCESS) {
        logError("Failed to Create Graphics Pipeline!");
        return false;
    }
    
    return true;
}

bool GraphicsPipeline::createDescriptorPool(size_t size) {
    if (this->device == nullptr) return false;

    std::vector<VkDescriptorPoolSize> poolSizes(3);

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(size);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(size);
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_TEXTURES * size);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(size);

    VkResult ret = vkCreateDescriptorPool(this->device, &poolInfo, nullptr, &this->descriptorPool);
    if (ret != VK_SUCCESS) {
       logError("Failed to Create Descriptor Pool!");
       return false;
    }
    
    return true;
}

bool GraphicsPipeline::createDescriptorSetLayout() {
    if (this->device == nullptr) return false;
    
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

    VkDescriptorSetLayoutBinding modelUniformLayoutBinding{};
    modelUniformLayoutBinding.binding = 0;
    modelUniformLayoutBinding.descriptorCount = 1;
    modelUniformLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    modelUniformLayoutBinding.pImmutableSamplers = nullptr;
    modelUniformLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layoutBindings.push_back(modelUniformLayoutBinding);

    VkDescriptorSetLayoutBinding ssboLayoutBinding{};
    ssboLayoutBinding.binding = 1;
    ssboLayoutBinding.descriptorCount = 1;
    ssboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    ssboLayoutBinding.pImmutableSamplers = nullptr;
    ssboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layoutBindings.push_back(ssboLayoutBinding);

    uint32_t numberOfTextures = Models::INSTANCE()->getTextures().size();
    VkDescriptorSetLayoutBinding samplersLayoutBinding{};
    samplersLayoutBinding.binding = 2;
    samplersLayoutBinding.descriptorCount = numberOfTextures > 0 ? numberOfTextures : 0;
    samplersLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplersLayoutBinding.pImmutableSamplers = nullptr;
    samplersLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layoutBindings.push_back(samplersLayoutBinding);

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = layoutBindings.size();
    layoutInfo.pBindings = layoutBindings.data();

    VkResult ret = vkCreateDescriptorSetLayout(this->device, &layoutInfo, nullptr, &this->descriptorSetLayout);
    if (ret != VK_SUCCESS) {
        logError("Failed to Create Descriptor Set Layout!");
        return false;
    }
    
    return true;
}

bool GraphicsPipeline::createSsboBufferFromModel(const VkPhysicalDevice & physicalDevice, const VkCommandPool & commandpool, const VkQueue & graphicsQueue, VkDeviceSize bufferSize, bool makeHostWritable)
{
    if (this->device == nullptr || bufferSize == 0) return true;

    if (this->ssboBuffer != nullptr) vkDestroyBuffer(this->device, this->ssboBuffer, nullptr);
    if (this->ssboBufferMemory != nullptr) vkFreeMemory(this->device, this->ssboBufferMemory, nullptr);

    if (!makeHostWritable) {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        if (!Helper::createBuffer(physicalDevice, this->device, bufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer, stagingBufferMemory)) {
            logError("Failed to get Create Staging Buffer");
            return false;
        }

        void* data = nullptr;
        vkMapMemory(this->device, stagingBufferMemory, 0, bufferSize, 0, &data);
        Helper::copyModelsContentIntoBuffer(data, SSBO, bufferSize);
        vkUnmapMemory(this->device, stagingBufferMemory);

        if (!Helper::createBuffer(physicalDevice, this->device, bufferSize,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                this->ssboBuffer, this->ssboBufferMemory)) {
            logError("Failed to get Create SSBO Buffer");
            return false;
        }

        Helper::copyBuffer(this->device, commandpool, graphicsQueue, stagingBuffer,this->ssboBuffer, bufferSize);

        vkDestroyBuffer(this->device, stagingBuffer, nullptr);
        vkFreeMemory(this->device, stagingBufferMemory, nullptr);        
    } else {
        if (!Helper::createBuffer(physicalDevice, this->device, bufferSize,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                this->ssboBuffer, this->ssboBufferMemory)) {
            logError("Failed to get Create Vertex Buffer");
            return false;
        }

        void * data = nullptr;
        vkMapMemory(this->device, ssboBufferMemory, 0, bufferSize, 0, &data);
        Helper::copyModelsContentIntoBuffer(data, SSBO, bufferSize);
        vkUnmapMemory(this->device, ssboBufferMemory);        
    }
    
    return true;
}

bool GraphicsPipeline::createTextureSampler(const VkPhysicalDevice & physicalDevice, VkSampler & sampler, VkSamplerAddressMode addressMode) {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = addressMode;
    samplerInfo.addressModeV = addressMode;
    samplerInfo.addressModeW = addressMode;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VkResult ret = vkCreateSampler(this->device, &samplerInfo, nullptr, &sampler);
    if (ret != VK_SUCCESS) {
        logError("Failed to Create Texture Sampler!");
        return false;
    }

    return true;
}

bool GraphicsPipeline::createDescriptorSets(size_t size) {
    if (this->device == nullptr) return false;
    
    std::vector<VkDescriptorSetLayout> layouts(size, this->descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = this->descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(size);
    allocInfo.pSetLayouts = layouts.data();

    this->descriptorSets.resize(size);
    VkResult ret = vkAllocateDescriptorSets(this->device, &allocInfo, this->descriptorSets.data());
    if (ret != VK_SUCCESS) {
        logError("Failed to Allocate Descriptor Sets!");
        return false;
    }

    std::map<std::string, std::unique_ptr<Texture>> & textures = Models::INSTANCE()->getTextures();
    uint32_t numberOfTextures = textures.size();        

    std::vector<VkDescriptorImageInfo> descriptorImageInfos;
    if (numberOfTextures > 0) {
        for (uint32_t i = 0; i < numberOfTextures; ++i) {
            VkDescriptorImageInfo texureDescriptorInfo = {};
            texureDescriptorInfo.sampler = this->textureSampler;
            texureDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            texureDescriptorInfo.imageView = Models::INSTANCE()->findTextureImageViewById(i);
            descriptorImageInfos.push_back(texureDescriptorInfo);
        }
    } else {
            VkDescriptorImageInfo texureDescriptorInfo = {};
            texureDescriptorInfo.sampler = nullptr;
            texureDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            texureDescriptorInfo.imageView = Models::INSTANCE()->findTextureImageViewById(-1);
            descriptorImageInfos.push_back(texureDescriptorInfo);        
    }

    VkDescriptorBufferInfo ssboBufferInfo{};
    ssboBufferInfo.buffer = this->ssboBuffer;
    ssboBufferInfo.offset = 0;
    ssboBufferInfo.range = Models::INSTANCE()->getModelsBufferSizes().ssboBufferSize;

    for (size_t i = 0; i < this->descriptorSets.size(); i++) {
        VkDescriptorBufferInfo uniformBufferInfo{};
        uniformBufferInfo.buffer = this->uniformBuffers[i];
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

        VkWriteDescriptorSet ssboDescriptorSet = {};
        ssboDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        ssboDescriptorSet.dstSet = this->descriptorSets[i];
        ssboDescriptorSet.dstBinding = 1;
        ssboDescriptorSet.dstArrayElement = 0;
        ssboDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        ssboDescriptorSet.descriptorCount = 1;
        ssboDescriptorSet.pBufferInfo = &ssboBufferInfo;
        descriptorWrites.push_back(ssboDescriptorSet);

        VkWriteDescriptorSet samplerDescriptorSet = {};
        samplerDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        samplerDescriptorSet.dstBinding = 2;
        samplerDescriptorSet.dstArrayElement = 0;
        samplerDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerDescriptorSet.descriptorCount = numberOfTextures > 0 ? numberOfTextures : 1;
        samplerDescriptorSet.pImageInfo = descriptorImageInfos.data();
        samplerDescriptorSet.dstSet = this->descriptorSets[i];
        descriptorWrites.push_back(samplerDescriptorSet);

        vkUpdateDescriptorSets(this->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
    
    return true;
}

std::vector<VkPipelineShaderStageCreateInfo> GraphicsPipeline::getShaderStageCreateInfos() {
    std::vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos;

    for (auto & shader : this->shaders) {
        if (shader.second != nullptr && shader.second->isValid()) {
            VkPipelineShaderStageCreateInfo shaderStageInfo{};
            
            shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageInfo.stage = shader.second->getShaderType();
            shaderStageInfo.module = shader.second->getShaderModule();
            shaderStageInfo.pName = "main";
            
            shaderCreateInfos.push_back(shaderStageInfo);    
        }
    }

    return shaderCreateInfos;
}

void GraphicsPipeline::draw(const VkCommandBuffer & commandBuffer) {
    
}

void GraphicsPipeline::updateUniformBuffers(const ModelUniforms & modelUniforms, const uint32_t & currentImage) {
    void* data;
    vkMapMemory(this->device, this->uniformBuffersMemory[currentImage], 0, sizeof(modelUniforms), 0, &data);
    memcpy(data, &modelUniforms, sizeof(modelUniforms));
    vkUnmapMemory(this->device, this->uniformBuffersMemory[currentImage]);    
}

void GraphicsPipeline::destroyPipelineObjects() {
    if (this->device == nullptr) return;

    if (this->pipeline != nullptr) {
        vkDestroyPipeline(this->device, this->pipeline, nullptr);
        this->pipeline = nullptr;
    }

    if (this->layout != nullptr) {
        vkDestroyPipelineLayout(this->device, this->layout, nullptr);
        this->layout = nullptr;
    }
    
    if (this->descriptorPool != nullptr) {
        vkDestroyDescriptorPool(this->device, this->descriptorPool, nullptr);
    }
}

GraphicsPipeline::~GraphicsPipeline() {
    for (auto & shader : this->shaders) {
        if (shader.second != nullptr) {
            delete shader.second;
            shader.second = nullptr;
        }
    }
    this->shaders.clear();
    
    this->destroyPipelineObjects();
}
