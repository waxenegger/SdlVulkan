#include "includes/graphics.h"

ModelsPipeline::ModelsPipeline(const Renderer * renderer) : GraphicsPipeline(renderer) { }

bool ModelsPipeline::createLocalBuffersFromModel() {
    if (this->renderer == nullptr || !this->renderer->isReady()) return false;
    
    BufferSummary bufferSizes = Models::INSTANCE()->getModelsBufferSizes(true);
    
    logError(std::to_string(bufferSizes.reservedSsboBufferSize));
    
    if (bufferSizes.vertexBufferSize == 0) return true;

    if (this->vertexBuffer != nullptr) vkDestroyBuffer(this->renderer->getLogicalDevice(), this->vertexBuffer, nullptr);
    if (this->vertexBufferMemory != nullptr) vkFreeMemory(this->renderer->getLogicalDevice(), this->vertexBufferMemory, nullptr);

    if (!Helper::createBuffer(this->renderer->getPhysicalDevice(), this->renderer->getLogicalDevice(), bufferSizes.reservedVertexBufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            this->vertexBuffer, this->vertexBufferMemory)) {
        logError("Failed to get Create Models Pipeline Vertex Buffer");
        return false;
    }

    void * data = nullptr;
    vkMapMemory(this->renderer->getLogicalDevice(), this->vertexBufferMemory, 0, bufferSizes.vertexBufferSize, 0, &data);
    Helper::copyModelsContentIntoBuffer(data, VERTEX, bufferSizes.vertexBufferSize);
    vkUnmapMemory(this->renderer->getLogicalDevice(), this->vertexBufferMemory);

    // indices
    if (bufferSizes.indexBufferSize == 0) return true;

    if (this->indexBuffer != nullptr) vkDestroyBuffer(this->renderer->getLogicalDevice(), this->indexBuffer, nullptr);
    if (this->indexBufferMemory != nullptr) vkFreeMemory(this->renderer->getLogicalDevice(), this->indexBufferMemory, nullptr);

    if (!Helper::createBuffer(this->renderer->getPhysicalDevice(), this->renderer->getLogicalDevice(), bufferSizes.reservedIndexBufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            this->indexBuffer, this->indexBufferMemory)) {
        logError("Failed to get Create Models Pipeline Index Buffer");
        return false;
    }

    data = nullptr;
    vkMapMemory(this->renderer->getLogicalDevice(), this->indexBufferMemory, 0, bufferSizes.indexBufferSize, 0, &data);
    Helper::copyModelsContentIntoBuffer(data, INDEX, bufferSizes.indexBufferSize);
    vkUnmapMemory(this->renderer->getLogicalDevice(), this->indexBufferMemory);

    return true;
}

bool ModelsPipeline::updateLocalModelBuffers() {
    if (this->renderer == nullptr || !this->renderer->isReady()) return false;
    
    BufferSummary bufferSizes = Models::INSTANCE()->getModelsBufferSizes(true);
     
    if (bufferSizes.vertexBufferSize == 0) return true;

    logError(std::to_string(bufferSizes.reservedIndexBufferSize));
    logError(std::to_string(bufferSizes.indexBufferSize));

    
    void * data = nullptr;
    vkMapMemory(this->renderer->getLogicalDevice(), this->vertexBufferMemory, 0, bufferSizes.vertexBufferSize, 0, &data);
    Helper::copyModelsContentIntoBuffer(data, VERTEX, bufferSizes.vertexBufferSize);
    vkUnmapMemory(this->renderer->getLogicalDevice(), this->vertexBufferMemory);

    // indices
    if (bufferSizes.indexBufferSize == 0) return true;

    data = nullptr;
    vkMapMemory(this->renderer->getLogicalDevice(), this->indexBufferMemory, 0, bufferSizes.indexBufferSize, 0, &data);
    Helper::copyModelsContentIntoBuffer(data, INDEX, bufferSizes.indexBufferSize);
    vkUnmapMemory(this->renderer->getLogicalDevice(), this->indexBufferMemory);

    return true;
}


bool ModelsPipeline::createDeviceBuffersFromModel() {
    if (this->renderer == nullptr || !this->renderer->isReady()) return false;
    
    BufferSummary bufferSizes = Models::INSTANCE()->getModelsBufferSizes(true);
     
    if (bufferSizes.vertexBufferSize == 0) return true;

    if (this->vertexBuffer != nullptr) vkDestroyBuffer(this->renderer->getLogicalDevice(), this->vertexBuffer, nullptr);
    if (this->vertexBufferMemory != nullptr) vkFreeMemory(this->renderer->getLogicalDevice(), this->vertexBufferMemory, nullptr);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    if (!Helper::createBuffer(this->renderer->getPhysicalDevice(), this->renderer->getLogicalDevice(), bufferSizes.vertexBufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory)) {
        logError("Failed to get Create Models Pipeline Staging Buffer");
        return false;
    }

    void* data = nullptr;
    vkMapMemory(this->renderer->getLogicalDevice(), stagingBufferMemory, 0, bufferSizes.vertexBufferSize, 0, &data);
    Helper::copyModelsContentIntoBuffer(data, VERTEX, bufferSizes.vertexBufferSize);
    vkUnmapMemory(this->renderer->getLogicalDevice(), stagingBufferMemory);

    if (!Helper::createBuffer(this->renderer->getPhysicalDevice(), this->renderer->getLogicalDevice(), bufferSizes.vertexBufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            this->vertexBuffer, this->vertexBufferMemory)) {
        logError("Failed to get Models Pipeline Create Vertex Buffer");
        return false;
    }

    Helper::copyBuffer(this->renderer->getLogicalDevice(), this->renderer->getCommandPool(), this->renderer->getGraphicsQueue(), stagingBuffer,this->vertexBuffer, bufferSizes.vertexBufferSize);

    vkDestroyBuffer(this->renderer->getLogicalDevice(), stagingBuffer, nullptr);
    vkFreeMemory(this->renderer->getLogicalDevice(), stagingBufferMemory, nullptr);
    
    // indices
    if (bufferSizes.indexBufferSize == 0) return true;

    if (this->indexBuffer != nullptr) vkDestroyBuffer(this->renderer->getLogicalDevice(), this->indexBuffer, nullptr);
    if (this->indexBufferMemory != nullptr) vkFreeMemory(this->renderer->getLogicalDevice(), this->indexBufferMemory, nullptr);

    if (!Helper::createBuffer(this->renderer->getPhysicalDevice(), this->renderer->getLogicalDevice(), bufferSizes.indexBufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory)) {
        logError("Failed to get Create Models Pipeline Staging Buffer");
        return false;
    }

    data = nullptr;
    vkMapMemory(this->renderer->getLogicalDevice(), stagingBufferMemory, 0, bufferSizes.indexBufferSize, 0, &data);
    Helper::copyModelsContentIntoBuffer(data, INDEX, bufferSizes.indexBufferSize);
    vkUnmapMemory(this->renderer->getLogicalDevice(), stagingBufferMemory);

    if (!Helper::createBuffer(this->renderer->getPhysicalDevice(), this->renderer->getLogicalDevice(), bufferSizes.indexBufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            this->indexBuffer, this->indexBufferMemory)) {
        logError("Failed to get Create Models Pipeline Index Buffer");
        return false;
    }
    
    Helper::copyBuffer(this->renderer->getLogicalDevice(), this->renderer->getCommandPool(), this->renderer->getGraphicsQueue(), stagingBuffer,this->indexBuffer, bufferSizes.indexBufferSize);

    vkDestroyBuffer(this->renderer->getLogicalDevice(), stagingBuffer, nullptr);
    vkFreeMemory(this->renderer->getLogicalDevice(), stagingBufferMemory, nullptr);
    
    return true;
}

void ModelsPipeline::prepareModelTextures() {    
    if (this->renderer == nullptr || !this->renderer->isReady()) return;
        
    auto & textures = Models::INSTANCE()->getTextures();
    
    // put in one dummy one to satify shader if we have none...
    if (textures.empty()) {
        Models::INSTANCE()->addDummyTexture(this->renderer->getSwapChainExtent());
    }

    for (auto & texture : textures) {
        if (!texture.second->isValid() || texture.second->getTextureImageView() != nullptr) continue;
        
        VkDeviceSize imageSize = texture.second->getSize();
        
        VkBuffer stagingBuffer = nullptr;
        VkDeviceMemory stagingBufferMemory = nullptr;
        if (!Helper::createBuffer(this->renderer->getPhysicalDevice(), this->renderer->getLogicalDevice(),
            imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory)) {
                logError("Failed to Create Models Pipeline Texture Staging Buffer");
                return;
        }

        void* data;
        vkMapMemory(this->renderer->getLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, texture.second->getPixels(), static_cast<size_t>(imageSize));
        vkUnmapMemory(this->renderer->getLogicalDevice(), stagingBufferMemory);
        
        VkImage textureImage = nullptr;
        VkDeviceMemory textureImageMemory = nullptr;
        
        if (!Helper::createImage(this->renderer->getPhysicalDevice(), this->renderer->getLogicalDevice(),
            texture.second->getWidth(), texture.second->getHeight(), 
            texture.second->getImageFormat(), 
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
            textureImage, textureImageMemory,1, MIPMAP_LEVELS)) {
                logError("Failed to Create Models Pipeline Texture Image");
                return;
        }

        Helper::transitionImageLayout(this->renderer->getLogicalDevice(), this->renderer->getCommandPool(), this->renderer->getGraphicsQueue(), 
            textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1, MIPMAP_LEVELS);
        Helper::copyBufferToImage(this->renderer->getLogicalDevice(), this->renderer->getCommandPool(), this->renderer->getGraphicsQueue(),
            stagingBuffer, textureImage, static_cast<uint32_t>(texture.second->getWidth()), static_cast<uint32_t>(texture.second->getHeight()));
        Helper::generateMipmaps(
            this->renderer->getLogicalDevice(), this->renderer->getCommandPool(), this->renderer->getGraphicsQueue(), textureImage, texture.second->getWidth(), texture.second->getHeight(), MIPMAP_LEVELS);

        vkDestroyBuffer(this->renderer->getLogicalDevice(), stagingBuffer, nullptr);
        vkFreeMemory(this->renderer->getLogicalDevice(), stagingBufferMemory, nullptr);
        
        if (textureImage != nullptr) texture.second->setTextureImage(textureImage);
        if (textureImageMemory != nullptr) texture.second->setTextureImageMemory(textureImageMemory);
        
        VkImageView textureImageView = Helper::createImageView(this->renderer->getLogicalDevice(), textureImage, texture.second->getImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT, 1, MIPMAP_LEVELS);
        if (textureImageView != nullptr) texture.second->setTextureImageView(textureImageView);
                
        texture.second->freeSurface();
    }
    
    logInfo("Number of Model Textures: " + std::to_string(textures.size()));
}

bool ModelsPipeline::createGraphicsPipeline(const VkPushConstantRange & pushConstantRange) {
    if (this->renderer == nullptr || !this->renderer->isReady()) return false;
    
    if (!this->createLocalBuffersFromModel()) {
        logError("Failed to create Buffers from Models");        
    }
    
    this->prepareModelTextures();

    // components SSBOs
    if (!this->createSsboBuffersFromComponents()) {
        logError("Failed to create Components Pipeline SSBO");
        return false;        
    }
    
    this->pushConstantRange = pushConstantRange;
    
    if (!this->createTextureSampler(VK_SAMPLER_ADDRESS_MODE_REPEAT)) {
        logError("Failed to create Models Pipeline Texture Sampler");
        return false;        
    }
        
    if (!this->createDescriptorPool()) {
        logError("Failed to create Models Pipeline Descriptor Pool");
        return false;
    }

    if (!this->createDescriptorSetLayout()) {
        logError("Failed to create Models Pipeline Descriptor Set Layout");
        return false;
    }

    if (!this->createDescriptorSets()) {
        logError("Failed to create Models Pipeline Descriptor Sets");
        return false;
    }

    if (!this->updateGraphicsPipeline()) return false;
    
    return true;
}

bool ModelsPipeline::updateGraphicsPipeline() {
    if (this->renderer == nullptr || !this->renderer->isReady() || this->descriptorSetLayout == nullptr) return false;
        
    this->destroyPipelineObjects();
    
    const std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos = this->getShaderStageCreateInfos();
    if (this->getShaderStageCreateInfos().size() < 2) {
        logError("Models Pipeline is missing required shaders");
        return false;
    }

    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    if (this->vertexBuffer != nullptr) {
        const VkVertexInputBindingDescription bindingDescription = ModelVertex::getBindingDescription();
        const std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions = ModelVertex::getAttributeDescriptions();

        vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
        vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
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
    
    if (this->pushConstantRange.size > 0) {
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    }
    
    VkResult ret = vkCreatePipelineLayout(this->renderer->getLogicalDevice(), &pipelineLayoutInfo, nullptr, &this->layout);
    if (ret != VK_SUCCESS) {
        logError("Failed to Create Models Pipeline Layout!");
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
        logError("Failed to Create Models Pipeline!");
        return false;
    }
    
    return true;
}

void ModelsPipeline::update() {
    this->updateSsboBuffersComponents();
}

void ModelsPipeline::draw(const VkCommandBuffer & commandBuffer, const uint16_t commandBufferIndex) {
    if (this->isReady() && this->isEnabled()) {
        if (this->vertexBuffer != nullptr) {
            vkCmdBindDescriptorSets(
                commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
                this->layout, 0, 1, &this->descriptorSets[commandBufferIndex], 0, nullptr);
            
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline);

            VkBuffer vertexBuffers[] = {this->vertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        }
            
        if (this->indexBuffer != nullptr) {
            vkCmdBindIndexBuffer(commandBuffer, this->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        }
        
        auto & allModels = Models::INSTANCE()->getModels();
        for (auto & model :  allModels) {            
            auto allComponents = Components::INSTANCE()->getAllComponentsForModel(model->getId());
            if (allComponents.empty()) continue;

            auto meshes = model->getMeshes();
            uint32_t instanceOffset = 0;
            
            for (Mesh & mesh : meshes) {

                if (!mesh.isBoundingBoxMesh() || this->isShowingBoundingBoxes()) {
                    for (auto & comp : allComponents) {
                        if (!comp->isVisible()) continue;


                            ModelProperties props = { comp->getModelMatrix()};
                            vkCmdPushConstants(
                                commandBuffer, this->layout,
                                VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(struct ModelProperties), &props);
                            
                            if (this->indexBuffer != nullptr) {
                                vkCmdDrawIndexed(commandBuffer, mesh.getIndices().size() , 1, mesh.getIndexOffset(), mesh.getVertexOffset(), comp->getSsboIndex() + instanceOffset);
                            } else {
                                vkCmdDraw(commandBuffer, mesh.getVertices().size(), 1, 0, comp->getSsboIndex() + instanceOffset);
                            }
                    }
                }
                
                instanceOffset++;
            }
        }
    }
}
    
bool ModelsPipeline::createDescriptorPool() {
    if (this->renderer == nullptr || !this->renderer->isReady()) return false;

    if (this->descriptorPool != nullptr) {
        vkDestroyDescriptorPool(this->renderer->getLogicalDevice(), this->descriptorPool, nullptr);
        this->descriptorPool = nullptr;
    }

    std::vector<VkDescriptorPoolSize> poolSizes(3);

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = this->renderer->getImageCount();
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = this->renderer->getImageCount();
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(this->renderer->getImageCount() * Models::INSTANCE()->getTextures().size());

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = this->renderer->getImageCount();

    VkResult ret = vkCreateDescriptorPool(this->renderer->getLogicalDevice(), &poolInfo, nullptr, &this->descriptorPool);
    if (ret != VK_SUCCESS) {
       logError("Failed to Create Models Pipeline Descriptor Pool!");
       return false;
    }
    
    return true;
}

bool ModelsPipeline::createDescriptorSetLayout() {
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

    VkResult ret = vkCreateDescriptorSetLayout(this->renderer->getLogicalDevice(), &layoutInfo, nullptr, &this->descriptorSetLayout);
    if (ret != VK_SUCCESS) {
        logError("Failed to Create Models Pipeline Descriptor Set Layout!");
        return false;
    }
    
    return true;
}

bool ModelsPipeline::createSsboBuffersFromComponents() {
    if (this->renderer == nullptr || !this->renderer->isReady()) return false;
   
    const BufferSummary bufferSizes = Models::INSTANCE()->getModelsBufferSizes(true);

    if (bufferSizes.reservedSsboBufferSize == 0) return true;

    if (this->ssboBuffer != nullptr) vkDestroyBuffer(this->renderer->getLogicalDevice(), this->ssboBuffer, nullptr);
    if (this->ssboBufferMemory != nullptr) vkFreeMemory(this->renderer->getLogicalDevice(), this->ssboBufferMemory, nullptr);

    if (!Helper::createBuffer(this->renderer->getPhysicalDevice(), this->renderer->getLogicalDevice(), bufferSizes.reservedSsboBufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            this->ssboBuffer, this->ssboBufferMemory)) {
        logError("Failed to get Create Models Pipeline Ssbo Buffer");
        return false;
    }

    void * data = nullptr;
    vkMapMemory(this->renderer->getLogicalDevice(), ssboBufferMemory, 0, bufferSizes.reservedSsboBufferSize, 0, &data);
    Helper::copyComponentsPropertiesIntoSsbo(data, bufferSizes.reservedSsboBufferSize);
    vkUnmapMemory(this->renderer->getLogicalDevice(), ssboBufferMemory);
    
    return true;
}

void ModelsPipeline::updateSsboBuffersComponents() {
    if (this->renderer == nullptr || !this->renderer->isReady() || this->ssboBufferMemory == nullptr) return;
   
    const BufferSummary bufferSizes = Models::INSTANCE()->getModelsBufferSizes();

    if (bufferSizes.reservedSsboBufferSize == 0) return;

    void * data = nullptr;
    vkMapMemory(this->renderer->getLogicalDevice(), this->ssboBufferMemory, 0, bufferSizes.reservedSsboBufferSize, 0, &data);
    Helper::copyComponentsPropertiesIntoSsbo(data, bufferSizes.reservedSsboBufferSize);
    vkUnmapMemory(this->renderer->getLogicalDevice(), this->ssboBufferMemory);
}

bool ModelsPipeline::createSsboBufferFromModel(VkDeviceSize bufferSize, bool makeHostWritable) {
    if (this->renderer == nullptr || !this->renderer->isReady()) return false;
   
    if (bufferSize == 0) return true;

    if (this->ssboBuffer != nullptr) vkDestroyBuffer(this->renderer->getLogicalDevice(), this->ssboBuffer, nullptr);
    if (this->ssboBufferMemory != nullptr) vkFreeMemory(this->renderer->getLogicalDevice(), this->ssboBufferMemory, nullptr);

    if (!makeHostWritable) {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        if (!Helper::createBuffer(this->renderer->getPhysicalDevice(), this->renderer->getLogicalDevice(), bufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer, stagingBufferMemory)) {
            logError("Failed to get Create Models Pipeline Staging Buffer");
            return false;
        }

        void* data = nullptr;
        vkMapMemory(this->renderer->getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
        Helper::copyModelsContentIntoBuffer(data, SSBO, bufferSize);
        vkUnmapMemory(this->renderer->getLogicalDevice(), stagingBufferMemory);

        if (!Helper::createBuffer(this->renderer->getPhysicalDevice(), this->renderer->getLogicalDevice(), bufferSize,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                this->ssboBuffer, this->ssboBufferMemory)) {
            logError("Failed to get Create Models Pipeline SSBO Buffer");
            return false;
        }

        Helper::copyBuffer(this->renderer->getLogicalDevice(), this->renderer->getCommandPool(), this->renderer->getGraphicsQueue(), stagingBuffer,this->ssboBuffer, bufferSize);

        vkDestroyBuffer(this->renderer->getLogicalDevice(), stagingBuffer, nullptr);
        vkFreeMemory(this->renderer->getLogicalDevice(), stagingBufferMemory, nullptr);        
    } else {
        if (!Helper::createBuffer(this->renderer->getPhysicalDevice(), this->renderer->getLogicalDevice(), bufferSize,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                this->ssboBuffer, this->ssboBufferMemory)) {
            logError("Failed to get Create Models Pipeline Vertex Buffer");
            return false;
        }

        void * data = nullptr;
        vkMapMemory(this->renderer->getLogicalDevice(), ssboBufferMemory, 0, bufferSize, 0, &data);
        Helper::copyModelsContentIntoBuffer(data, SSBO, bufferSize);
        vkUnmapMemory(this->renderer->getLogicalDevice(), ssboBufferMemory);        
    }
    
    return true;
}

bool ModelsPipeline::createDescriptorSets() {
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
        logError("Failed to Allocate Models Pipeline Descriptor Sets!");
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
    ssboBufferInfo.range = Models::INSTANCE()->getModelsBufferSizes().reservedSsboBufferSize;
        
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
        samplerDescriptorSet.descriptorCount = numberOfTextures;
        samplerDescriptorSet.pImageInfo = descriptorImageInfos.data();
        samplerDescriptorSet.dstSet = this->descriptorSets[i];
        descriptorWrites.push_back(samplerDescriptorSet);

        vkUpdateDescriptorSets(this->renderer->getLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
    
    return true;
}


