#include "includes/graphics.h"

GraphicsPipeline::GraphicsPipeline(const VkPhysicalDevice physicalDevice, const int queueIndex) {
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    VkDeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.flags = 0;
    queueCreateInfo.pNext = nullptr;
    queueCreateInfo.queueFamilyIndex = queueIndex;
    queueCreateInfo.queueCount = 2;
    const float priority = 1.0f;
    queueCreateInfo.pQueuePriorities = &priority;

    queueCreateInfos.push_back(queueCreateInfo);

    const std::vector<const char * > extensionsToEnable = { 
        "VK_KHR_swapchain"
    };

    VkPhysicalDeviceFeatures deviceFeatures {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.multiDrawIndirect = VK_TRUE;
    deviceFeatures.fillModeNonSolid = VK_TRUE;

    VkDeviceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.ppEnabledExtensionNames = extensionsToEnable.data();
    createInfo.enabledExtensionCount = extensionsToEnable.size();

    const VkResult ret = vkCreateDevice(physicalDevice, &createInfo, nullptr, &this->device);
    if (ret != VK_SUCCESS) {
        logError("Failed to create Logical Device!");
        return;
    }

    this->graphicsQueueIndex = this->presentQueueIndex = queueCreateInfos[0].queueFamilyIndex;

    vkGetDeviceQueue(this->device, this->graphicsQueueIndex , 0, &this->graphicsQueue);
    vkGetDeviceQueue(this->device, this->presentQueueIndex , 0, &this->presentQueue);    
}

void GraphicsPipeline::addShader(const std::string & filename, const VkShaderStageFlagBits & shaderType) {
    const std::map<std::string, std::unique_ptr<Shader>>::iterator existingShader = this->shaders.find(filename);
    if (existingShader != this->shaders.end()) {
        logInfo("Shader " + filename + " already exists!");
        return;
    }
    
    std::unique_ptr<Shader> newShader = std::make_unique<Shader>(this->device, filename, shaderType);
    if (newShader->isValid()) this->shaders[filename] = std::move(newShader);
}

GraphicsPipeline::~GraphicsPipeline() {
    this->shaders.clear();
    
    if (this->device != nullptr) vkDestroyDevice(this->device, nullptr);
}
