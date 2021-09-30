#include "includes/graphics.h"

GraphicsPipeline::GraphicsPipeline(const Renderer * renderer) : renderer(renderer) { }

void GraphicsPipeline::addShader(const std::string & filename, const VkShaderStageFlagBits & shaderType) {
    const std::map<std::string, const Shader *>::iterator existingShader = this->shaders.find(filename);
    if (existingShader != this->shaders.end()) {
        logInfo("Shader " + filename + " already exists!");
        return;
    }
    
    std::unique_ptr<Shader> newShader = std::make_unique<Shader>(this->renderer->getLogicalDevice(), filename, shaderType);
    if (newShader->isValid()) this->shaders[filename] = newShader.release();
}

bool GraphicsPipeline::createTextureSampler(VkSamplerAddressMode addressMode) {
    if (this->renderer->getLogicalDevice() == nullptr) return false;
    
    if (this->textureSampler != nullptr) {
        vkDestroySampler(this->renderer->getLogicalDevice(), this->textureSampler, nullptr);
        this->textureSampler = nullptr;
    }

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(this->renderer->getPhysicalDevice(), &properties);

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

    VkResult ret = vkCreateSampler(this->renderer->getLogicalDevice(), &samplerInfo, nullptr, &this->textureSampler);
    if (ret != VK_SUCCESS) {
        logError("Failed to Create Texture Sampler!");
        return false;
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

void GraphicsPipeline::destroyPipelineObjects() {
    if (this->renderer->getLogicalDevice() == nullptr) return;

    if (this->pipeline != nullptr) {
        vkDestroyPipeline(this->renderer->getLogicalDevice(), this->pipeline, nullptr);
        this->pipeline = nullptr;
    }

    if (this->layout != nullptr) {
        vkDestroyPipelineLayout(this->renderer->getLogicalDevice(), this->layout, nullptr);
        this->layout = nullptr;
    }
}

bool GraphicsPipeline::isReady() {
    return this->pipeline != nullptr;
}

bool GraphicsPipeline::canRender() {
    uint8_t validShaders = 0;
    
    for (auto & shader : this->shaders) {
        if (shader.second->isValid()) validShaders++;
    }    
    
    return this->isReady() && validShaders >= 2;
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
        vkDestroySampler(this->renderer->getLogicalDevice(), this->textureSampler, nullptr);
        this->textureSampler = nullptr;
    }
    
    if (this->descriptorSetLayout != nullptr) {
        vkDestroyDescriptorSetLayout(this->renderer->getLogicalDevice(), this->descriptorSetLayout, nullptr);
        this->descriptorSetLayout = nullptr;
    }

    if (this->descriptorPool != nullptr) {
        vkDestroyDescriptorPool(this->renderer->getLogicalDevice(), this->descriptorPool, nullptr);
        this->descriptorPool = nullptr;
    }
    
    if (this->vertexBuffer != nullptr) vkDestroyBuffer(this->renderer->getLogicalDevice(), this->vertexBuffer, nullptr);
    if (this->vertexBufferMemory != nullptr) vkFreeMemory(this->renderer->getLogicalDevice(), this->vertexBufferMemory, nullptr);

    if (this->indexBuffer != nullptr) vkDestroyBuffer(this->renderer->getLogicalDevice(), this->indexBuffer, nullptr);
    if (this->indexBufferMemory != nullptr) vkFreeMemory(this->renderer->getLogicalDevice(), this->indexBufferMemory, nullptr);

    if (this->ssboBuffer != nullptr) vkDestroyBuffer(this->renderer->getLogicalDevice(), this->ssboBuffer, nullptr);
    if (this->ssboBufferMemory != nullptr) vkFreeMemory(this->renderer->getLogicalDevice(), this->ssboBufferMemory, nullptr);
}
