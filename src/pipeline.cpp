#include "includes/graphics.h"

GraphicsPipeline::GraphicsPipeline(const VkDevice & device) : device(device) { }

void GraphicsPipeline::addShader(const std::string & filename, const VkShaderStageFlagBits & shaderType) {
    const std::map<std::string, const Shader *>::iterator existingShader = this->shaders.find(filename);
    if (existingShader != this->shaders.end()) {
        logInfo("Shader " + filename + " already exists!");
        return;
    }
    
    std::unique_ptr<Shader> newShader = std::make_unique<Shader>(this->device, filename, shaderType);
    if (newShader->isValid()) this->shaders[filename] = newShader.release();
}

bool GraphicsPipeline::createTextureSampler(const VkPhysicalDevice & physicalDevice, VkSampler & sampler, VkSamplerAddressMode addressMode) {
    if (this->device == nullptr) return false;
    
    if (this->textureSampler != nullptr) {
        vkDestroySampler(this->device, this->textureSampler, nullptr);
        this->textureSampler = nullptr;
    }

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

bool GraphicsPipeline::createUniformBuffers(const VkPhysicalDevice & physicalDevice, size_t size) {
    
    for (size_t i = 0; i < this->uniformBuffers.size(); i++) {
        if (this->uniformBuffers[i] != nullptr) vkDestroyBuffer(this->device, this->uniformBuffers[i], nullptr);
    }
    for (size_t i = 0; i < this->uniformBuffersMemory.size(); i++) {
        if (this->uniformBuffersMemory[i] != nullptr) vkFreeMemory(this->device, this->uniformBuffersMemory[i], nullptr);
    }
    this->uniformBuffers.clear();
    this->uniformBuffersMemory.clear();
    
    VkDeviceSize bufferSize = sizeof(struct ModelUniforms);

    this->uniformBuffers.resize(size);
    this->uniformBuffersMemory.resize(size);

    for (size_t i = 0; i < this->uniformBuffers.size(); i++) {
        Helper::createBuffer(physicalDevice, this->device,
            bufferSize, 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            this->uniformBuffers[i], this->uniformBuffersMemory[i]);
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
}

bool GraphicsPipeline::isReady() {
    return this->pipeline != nullptr;
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
    
    if (this->textureSampler != nullptr) {
        vkDestroySampler(this->device, this->textureSampler, nullptr);
        this->textureSampler = nullptr;
    }
    
    if (this->descriptorSetLayout != nullptr) {
        vkDestroyDescriptorSetLayout(this->device, this->descriptorSetLayout, nullptr);
        this->descriptorSetLayout = nullptr;
    }

    if (this->descriptorPool != nullptr) {
        vkDestroyDescriptorPool(this->device, this->descriptorPool, nullptr);
        this->descriptorPool = nullptr;
    }
    
    if (this->vertexBuffer != nullptr) vkDestroyBuffer(this->device, this->vertexBuffer, nullptr);
    if (this->vertexBufferMemory != nullptr) vkFreeMemory(this->device, this->vertexBufferMemory, nullptr);

    if (this->indexBuffer != nullptr) vkDestroyBuffer(this->device, this->indexBuffer, nullptr);
    if (this->indexBufferMemory != nullptr) vkFreeMemory(this->device, this->indexBufferMemory, nullptr);

    if (this->ssboBuffer != nullptr) vkDestroyBuffer(this->device, this->ssboBuffer, nullptr);
    if (this->ssboBufferMemory != nullptr) vkFreeMemory(this->device, this->ssboBufferMemory, nullptr);
    
    for (size_t i = 0; i < this->uniformBuffers.size(); i++) {
        if (this->uniformBuffers[i] != nullptr) vkDestroyBuffer(this->device, this->uniformBuffers[i], nullptr);
    }
    for (size_t i = 0; i < this->uniformBuffersMemory.size(); i++) {
        if (this->uniformBuffersMemory[i] != nullptr) vkFreeMemory(this->device, this->uniformBuffersMemory[i], nullptr);
    }
}
