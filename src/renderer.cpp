#include "includes/graphics.h"

Renderer:: Renderer(const GraphicsContext * graphicsContext, const VkPhysicalDevice & physicalDevice, const int & queueIndex) : graphicsContext(graphicsContext), physicalDevice(physicalDevice), queueIndex(queueIndex) {
    
    this->graphicsQueueIndex = this->presentQueueIndex = queueIndex;
    
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

    const VkResult ret = vkCreateDevice(this->physicalDevice, &createInfo, nullptr, &this->logicalDevice);
    if (ret != VK_SUCCESS) {
        logError("Failed to create Logical Device!");
        return;
    }
    
    vkGetDeviceQueue(this->logicalDevice, this->graphicsQueueIndex , 0, &this->graphicsQueue);
    vkGetDeviceQueue(this->logicalDevice, this->presentQueueIndex , 0, &this->presentQueue);
}

bool Renderer::isReady() const {
    return this->graphicsContext != nullptr && this->physicalDevice != nullptr && this->logicalDevice  != nullptr;
}

bool Renderer::hasAtLeastOneActivePipeline() const {
    bool isReady = false;
    
    for (GraphicsPipeline * pipeline : this->pipelines) {
        if (pipeline != nullptr && pipeline->canRender()) {
            isReady = true;
            break;
        }
    }
    
    return isReady;
}

bool Renderer::createUniformBuffers() {
    if (!this->isReady()) return false;
    
    VkDeviceSize bufferSize = sizeof(struct ModelUniforms);

    this->uniformBuffers.resize(this->imageCount);
    this->uniformBuffersMemory.resize(this->imageCount);

    for (size_t i = 0; i < this->uniformBuffers.size(); i++) {
        Helper::createBuffer(this->physicalDevice, this->logicalDevice, bufferSize, 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            this->uniformBuffers[i], this->uniformBuffersMemory[i]);
    }

    return true;
}

void Renderer::updateUniformBuffers(const ModelUniforms & modelUniforms, const uint32_t & currentImage) {
    void* data;
    vkMapMemory(this->logicalDevice, this->uniformBuffersMemory[currentImage], 0, sizeof(modelUniforms), 0, &data);
    memcpy(data, &modelUniforms, sizeof(modelUniforms));
    vkUnmapMemory(this->logicalDevice, this->uniformBuffersMemory[currentImage]);    
}

const VkBuffer Renderer::getUniformBuffer(uint8_t index) const {
    if (index >= this->uniformBuffers.size()) return nullptr;
    
    return this->uniformBuffers[index];
}

GraphicsPipeline * Renderer::getPipeline(uint8_t index) {
    if (index >= this->pipelines.size()) return nullptr;
    
    return this->pipelines[index];
}

bool Renderer::canRender() const {
    return this->isReady() && this->hasAtLeastOneActivePipeline() && this->swapChain != nullptr && this->swapChainImages.size() == this->imageCount && 
        this->swapChainImages.size() == this->swapChainImageViews.size() && this->imagesInFlight.size() == this->swapChainImages.size() &&
        this->imageAvailableSemaphores.size() == this->imageCount && this->renderFinishedSemaphores.size() == this->imageCount && this->inFlightFences.size() == this->imageCount &&
        this->swapChainFramebuffers.size() == this->swapChainImages.size() && this->depthImages.size() == this->swapChainImages.size() && 
        this->depthImagesMemory.size() == this->swapChainImages.size() && this->depthImagesView.size() == this->swapChainImages.size() && 
        this->commandPool != nullptr;
}


uint8_t Renderer::addPipeline(GraphicsPipeline * pipeline) {
    if (!this->isReady()) {
        logError("Render has not been properly initialized!");
        return 0;
    }
        
    this->pipelines.push_back(pipeline);
    
    return this->pipelines.size()-1;
}

void Renderer::removePipeline(const uint8_t optIndexToRemove) {
    if (!this->isReady()) {
        logError("Render has not been properly initialized!");
        return;
    }
        
    if (optIndexToRemove < 0|| optIndexToRemove >= this->pipelines.size()) return;
    
    this->stopCommandBufferQueue();
    vkDeviceWaitIdle(this->logicalDevice);

    delete this->pipelines[optIndexToRemove];
}


bool Renderer::createRenderPass() {
    if (!this->isReady()) {
        logError("Renderer has not been initialized!");
        return false;
    }
    
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = SWAP_CHAIN_IMAGE_FORMAT.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkFormat depthFormat;
    if (!GraphicsContext::findDepthFormat(this->physicalDevice, depthFormat)) {
        logError("Failed to Find Depth Format!");        
    }
    
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkResult ret = vkCreateRenderPass(this->logicalDevice, &renderPassInfo, nullptr, &this->renderPass);
    if (ret != VK_SUCCESS) {
       logError("Failed to Create Render Pass!");
       return false;
    }
    
    return true;
}

bool Renderer::createSwapChain() {
    if (!this->isReady()) {
        logError("Renderer has not been initialized!");
        return false;
    }

    const std::vector<VkPresentModeKHR> presentModes = this->graphicsContext->queryDeviceSwapModes(this->physicalDevice);
    if (presentModes.empty()) {
        logError("Swap Modes Require Surface!");
        return false;
    }

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    if (!this->graphicsContext->getSurfaceCapabilities(this->physicalDevice, surfaceCapabilities)) return false;

    this->swapChainExtent = this->graphicsContext->getSwapChainExtent(surfaceCapabilities);

    if (surfaceCapabilities.maxImageCount > 0 && this->imageCount > surfaceCapabilities.maxImageCount) {
        this->imageCount = surfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.surface = this->graphicsContext->getVulkanSurface();
    createInfo.minImageCount = this->imageCount;
    createInfo.imageFormat = SWAP_CHAIN_IMAGE_FORMAT.format;
    createInfo.imageColorSpace = SWAP_CHAIN_IMAGE_FORMAT.colorSpace;
    createInfo.imageExtent = this->swapChainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.preTransform = surfaceCapabilities.currentTransform;
    createInfo.presentMode = Helper::pickBestDeviceSwapMode(presentModes);
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    const uint32_t queueFamilyIndices[] = { this->graphicsQueueIndex, this->presentQueueIndex };
    if (this->graphicsQueueIndex != this->presentQueueIndex) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = this->imageCount;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    VkResult ret = vkCreateSwapchainKHR(this->logicalDevice, &createInfo, nullptr, &this->swapChain);
    if (ret != VK_SUCCESS) {
        logError("Failed to Creat Swap Chain!");
        return false;
    }

    ret = vkGetSwapchainImagesKHR(this->logicalDevice, this->swapChain, &this->imageCount, nullptr);
    if (ret != VK_SUCCESS) {
        logError("Failed to Get Swap Chain Image Count!");
        return false;
    }

    logInfo("Buffering: " + std::to_string(this->imageCount));
    this->swapChainImages.resize(this->imageCount);

    ret = vkGetSwapchainImagesKHR(this->logicalDevice, this->swapChain, &this->imageCount, this->swapChainImages.data());
    if (ret != VK_SUCCESS) {
        logError("Failed to Create Swap Chain Images!");
        return false;
    }

    return true;
}

bool Renderer::createImageViews() {
    if (!this->isReady()) {
        logError("Renderer has not been initialized!");
        return false;
    }

    this->swapChainImageViews.resize(this->swapChainImages.size());

    for (size_t i = 0; i < this->swapChainImages.size(); i++) {
        VkImageView imgView = 
            Helper::createImageView(this->logicalDevice, this->swapChainImages[i], SWAP_CHAIN_IMAGE_FORMAT.format, VK_IMAGE_ASPECT_COLOR_BIT);
        if (imgView == nullptr) {
            logError("Failed to Create Swap Chain Image View!");
            return false;
        }
        
        this->swapChainImageViews[i] = imgView;
    }

    return true;
}

bool Renderer::createSyncObjects() {
    if (!this->isReady()) {
        logError("Renderer has not been initialized!");
        return false;
    }

    this->imageAvailableSemaphores.resize(this->imageCount);
    this->renderFinishedSemaphores.resize(this->imageCount);
    this->inFlightFences.resize(this->imageCount);
    this->imagesInFlight.resize(this->swapChainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < this->imageCount; i++) {

        if (vkCreateSemaphore(this->logicalDevice, &semaphoreInfo, nullptr, &this->imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(this->logicalDevice, &semaphoreInfo, nullptr, &this->renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(this->logicalDevice, &fenceInfo, nullptr, &this->inFlightFences[i]) != VK_SUCCESS) {
            logError("Failed to Create Synchronization Objects For Frame!");
            return false;
        }
    }

    return true;
}

bool Renderer::createCommandPool() {
    if (!this->isReady()) {
        logError("Renderer has not been initialized!");
        return false;
    }
    
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = this->graphicsQueueIndex;

    VkResult ret = vkCreateCommandPool(this->logicalDevice, &poolInfo, nullptr, &this->commandPool);
    if (ret != VK_SUCCESS) {
        logError("Failed to Create Command Pool!");
        return false;
    }

    return true;
}

VkCommandPool Renderer::getCommandPool() const {
    return this->commandPool;
}

VkQueue Renderer::getGraphicsQueue() const {
    return this->graphicsQueue;
}

bool Renderer::createFramebuffers() {
    if (!this->isReady()) {
        logError("Renderer has not been initialized!");
        return false;
    }

    this->swapChainFramebuffers.resize(this->swapChainImageViews.size());

     for (size_t i = 0; i < this->swapChainImageViews.size(); i++) {
         std::array<VkImageView, 2> attachments = {
             this->swapChainImageViews[i], this->depthImagesView[i]
         };

         VkFramebufferCreateInfo framebufferInfo{};
         framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
         framebufferInfo.renderPass = renderPass;
         framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
         framebufferInfo.pAttachments = attachments.data();
         framebufferInfo.width = swapChainExtent.width;
         framebufferInfo.height = swapChainExtent.height;
         framebufferInfo.layers = 1;

         VkResult ret = vkCreateFramebuffer(this->logicalDevice, &framebufferInfo, nullptr, &this->swapChainFramebuffers[i]);
         if (ret != VK_SUCCESS) {
            logError("Failed to Create Frame Buffers!");
            return false;
         }
     }

     return true;
}

bool Renderer::createDepthResources() {
    this->depthImages.resize(this->swapChainImages.size());
    this->depthImagesMemory.resize(this->depthImages.size());
    this->depthImagesView.resize(this->depthImages.size());
    
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    if (!GraphicsContext::findDepthFormat(this->physicalDevice, depthFormat)) {
        logError("Faild to create Depth Format!");
        return false;
    };

    for (uint16_t i=0;i<this->depthImages.size();i++) {
        if (!Helper::createImage(this->physicalDevice, this->logicalDevice, swapChainExtent.width, swapChainExtent.height, depthFormat, 
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                this->depthImages[i], this->depthImagesMemory[i])) {
            logError("Failed to create Depth Image!");
            return false;
        }
    
        VkImageView imgView = Helper::createImageView(this->logicalDevice, this->depthImages[i], depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
        if (imgView == nullptr) {
            logError("Failed to Create Depth Image View!");
            return false;
        }

        this->depthImagesView[i] = imgView;
    }
    
    return true;
}

void Renderer::initRenderer() {
    if (!this->updateRenderer()) return;
    if (!this->createCommandPool()) return;
    if (!this->createSyncObjects()) return;
    if (!this->createUniformBuffers()) return;
}

void Renderer::destroyRendererObjects() {
    this->destroySwapChainObjects();
    
    if (this->logicalDevice == nullptr) return;
    
    for (size_t i = 0; i < this->uniformBuffers.size(); i++) {
        if (this->uniformBuffers[i] != nullptr) vkDestroyBuffer(this->logicalDevice, this->uniformBuffers[i], nullptr);
    }
    for (size_t i = 0; i < this->uniformBuffersMemory.size(); i++) {
        if (this->uniformBuffersMemory[i] != nullptr) vkFreeMemory(this->logicalDevice, this->uniformBuffersMemory[i], nullptr);
    }
    
    for (GraphicsPipeline * pipeline : this->pipelines) {
        if (pipeline != nullptr) {
            delete pipeline;
            pipeline = nullptr;
        }
    }
    pipelines.clear();
    
    for (size_t i = 0; i < this->imageCount; i++) {
        if (i < this->renderFinishedSemaphores.size()) {
            if (this->renderFinishedSemaphores[i] != nullptr) {
                vkDestroySemaphore(this->logicalDevice, this->renderFinishedSemaphores[i], nullptr);
            }
        }

        if (i < this->imageAvailableSemaphores.size()) {
            if (this->imageAvailableSemaphores[i] != nullptr) {
                vkDestroySemaphore(this->logicalDevice, this->imageAvailableSemaphores[i], nullptr);
            }
        }
        
        if (i < this->inFlightFences.size()) {
            if (this->inFlightFences[i] != nullptr) {
                vkDestroyFence(this->logicalDevice, this->inFlightFences[i], nullptr);
            }
        }
    }
            
    this->renderFinishedSemaphores.clear();
    this->imageAvailableSemaphores.clear();
    this->imagesInFlight.clear();
    this->inFlightFences.clear();

    if (this->logicalDevice != nullptr && this->commandPool != nullptr) {
        vkDestroyCommandPool(this->logicalDevice, this->commandPool, nullptr);
        this->commandPool = nullptr;
    }
    
    if (this->logicalDevice != nullptr) {
        Models::INSTANCE()->cleanUpTextures(this->logicalDevice);
    }
}

void Renderer::destroySwapChainObjects() {
    if (this->logicalDevice == nullptr) return;

    vkDeviceWaitIdle(this->logicalDevice);
    
    for (uint16_t j=0;j<this->depthImages.size();j++) {
        if (this->depthImagesView[j] != nullptr) {
            vkDestroyImageView(this->logicalDevice, this->depthImagesView[j], nullptr);
            this->depthImagesView[j] = nullptr;            
        }
        if (this->depthImages[j] != nullptr) {
            vkDestroyImage(this->logicalDevice, this->depthImages[j], nullptr);
            this->depthImages[j] = nullptr;            
        }
        if (this->depthImagesMemory[j] != nullptr) {
            vkFreeMemory(this->logicalDevice, this->depthImagesMemory[j], nullptr);
            this->depthImagesMemory[j] = nullptr;            
        }
    }
    this->depthImages.clear();
    this->depthImagesView.clear();
    this->depthImagesMemory.clear();

    for (auto & framebuffer : this->swapChainFramebuffers) {
        if (framebuffer != nullptr) {
            vkDestroyFramebuffer(this->logicalDevice, framebuffer, nullptr);
            framebuffer = nullptr;
        }
    }
    this->swapChainFramebuffers.clear();

    if (this->commandPool != nullptr) {
        vkResetCommandPool(this->logicalDevice, this->commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
    }

    for (GraphicsPipeline * pipeline : this->pipelines) {
        if (pipeline != nullptr) {
            pipeline->destroyPipelineObjects();
        }
    }

    if (this->renderPass != nullptr) {
        vkDestroyRenderPass(this->logicalDevice, this->renderPass, nullptr);
        this->renderPass = nullptr;
    }
    
    for (uint16_t j=0;j<this->swapChainImages.size();j++) {
        if (this->swapChainImageViews[j] != nullptr) {
            vkDestroyImageView(this->logicalDevice, this->swapChainImageViews[j], nullptr);
            this->swapChainImageViews[j] = nullptr;
        }
    }
    this->swapChainImageViews.clear();

    if (this->swapChain != nullptr) {
        vkDestroySwapchainKHR(this->logicalDevice, this->swapChain, nullptr);
        this->swapChain = nullptr;
    }

}

VkDevice Renderer::getLogicalDevice() const {
    return this->logicalDevice;
}

VkPhysicalDevice Renderer::getPhysicalDevice() const {
    return this->physicalDevice;
}

void Renderer::startCommandBufferQueue() {
    if (!this->canRender() || this->workerQueue.isRunning()) return;
    
    this->workerQueue.startQueue(
        std::bind(&Renderer::createCommandBuffer, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&Renderer::destroyCommandBuffer , this, std::placeholders::_1), 
        this->swapChainFramebuffers.size());
}

void Renderer::stopCommandBufferQueue() {
    this->workerQueue.stopQueue();
}

void Renderer::destroyCommandBuffer(VkCommandBuffer commandBuffer) {
    if (!this->isReady()) return; 
    
    vkFreeCommandBuffers(this->logicalDevice, this->commandPool, 1, &commandBuffer);
}

VkCommandBuffer Renderer::createCommandBuffer(uint16_t commandBufferIndex, const bool threaded) {
    if (this->requiresRenderUpdate) return nullptr;
    
    std::vector<VkCommandBuffer> commandBuffers;
    
    // TODO: fork for threaded
    
    VkCommandBuffer commandBuffer = Helper::beginCommandBuffer(this->logicalDevice,this->commandPool);
    if (this->requiresRenderUpdate || commandBuffer == nullptr) return nullptr;
    
    commandBuffers.push_back(commandBuffer);
    
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = this->renderPass;
    renderPassInfo.framebuffer = this->swapChainFramebuffers[commandBufferIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = this->swapChainExtent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        
    for (GraphicsPipeline * pipeline : this->pipelines) {
        if (!this->requiresRenderUpdate && pipeline->canRender()) {
            pipeline->draw(commandBuffers, commandBufferIndex);
        }
    }
    
    vkCmdEndRenderPass(commandBuffer);

    if (!Helper::endCommandBuffer(commandBuffer)) return nullptr;

    return commandBuffer;
}

void Renderer::updateUniformBuffer(uint32_t currentImage) {
    ModelUniforms modelUniforms {};
    modelUniforms.camera = glm::vec4(Camera::INSTANCE()->getPosition(),1);
    modelUniforms.sun = glm::vec4(0.0f, 100.0f, 100.0f, 1);
    modelUniforms.viewMatrix = Camera::INSTANCE()->getViewMatrix();
    modelUniforms.projectionMatrix = Camera::INSTANCE()->getProjectionMatrix();

    this->updateUniformBuffers(modelUniforms, currentImage);
}

bool Renderer::createCommandBuffers() {
    this->commandBuffers.resize(this->swapChainFramebuffers.size());
    
    this->startCommandBufferQueue();

    return true;
}

void Renderer::drawFrame() {
    std::chrono::high_resolution_clock::time_point frameStart = std::chrono::high_resolution_clock::now();
    
    if (this->requiresRenderUpdate) {
        this->updateRenderer();
        return;
    }

    VkResult ret = vkWaitForFences(this->logicalDevice, 1, &this->inFlightFences[this->currentFrame], VK_TRUE, UINT64_MAX);
    if (ret != VK_SUCCESS) {
        this->requiresRenderUpdate = true;
        return;
    }
    
    uint32_t imageIndex;
    ret = vkAcquireNextImageKHR(
        this->logicalDevice, this->swapChain, IMAGE_ACQUIRE_TIMEOUT, this->imageAvailableSemaphores[this->currentFrame], VK_NULL_HANDLE, &imageIndex);
    
    if (ret != VK_SUCCESS) {
        if (ret != VK_ERROR_OUT_OF_DATE_KHR) {
            logError("Failed to Acquire Next Image");
        }
        this->requiresRenderUpdate = true;
        return;
    }

    if (this->commandBuffers[imageIndex] != nullptr) {
        this->workerQueue.queueCommandBufferForDeletion(this->commandBuffers[imageIndex]);
    }

    VkCommandBuffer latestCommandBuffer = this->workerQueue.getNextCommandBuffer(imageIndex);
    std::chrono::high_resolution_clock::time_point nextBufferFetchStart = std::chrono::high_resolution_clock::now();
    while (latestCommandBuffer == nullptr) {
        std::chrono::duration<double, std::milli> fetchPeriod = std::chrono::high_resolution_clock::now() - nextBufferFetchStart;
        if (fetchPeriod.count() > 500) {
            logInfo("Could not get new buffer for quite a while!");
            break;
        }
        latestCommandBuffer = this->workerQueue.getNextCommandBuffer(imageIndex);
    }
    std::chrono::duration<double, std::milli> timer = std::chrono::high_resolution_clock::now() - nextBufferFetchStart;
    //if (timer.count() < 50) {
    //    std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(50 - timer.count()));
    //}
    //timer = std::chrono::high_resolution_clock::now() - nextBufferFetchStart;
    //std::cout << "Fetch Time: " << timer.count() << " | " << this->workerQueue.getNumberOfItems(imageIndex) << std::endl;
    
    if (latestCommandBuffer == nullptr) return;
    this->commandBuffers[imageIndex] = latestCommandBuffer;

    this->updateUniformBuffer(imageIndex);
        
    if (this->imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        ret = vkWaitForFences(this->logicalDevice, 1, &this->imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        if (ret != VK_SUCCESS) {
             logError("vkWaitForFences 2 Failed");
        }
    }
    this->imagesInFlight[imageIndex] = this->inFlightFences[this->currentFrame];

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {this->imageAvailableSemaphores[this->currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = this->commandBuffers.empty() ? 0 : 1;
    submitInfo.pCommandBuffers = &this->commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {this->renderFinishedSemaphores[this->currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    ret = vkResetFences(this->logicalDevice, 1, &this->inFlightFences[this->currentFrame]);
    if (ret != VK_SUCCESS) {
        logError("Failed to Reset Fence!");
    }

    ret = vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, this->inFlightFences[this->currentFrame]);
    if (ret != VK_SUCCESS) {
        logError("Failed to Submit Draw Command Buffer!");
    }
    
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {this->swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    ret = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (ret != VK_SUCCESS) {
        if (ret != VK_ERROR_OUT_OF_DATE_KHR) {
            logError("Failed to Present Swap Chain Image!");
        }
        return;
    }
    
    this->currentFrame = (this->currentFrame + 1) % this->imageCount;

    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> time_span = now -frameStart;
    this->deltaTime = time_span.count();
    
    int frameRate = static_cast<int>(1000 / this->deltaTime);
    std::cout << frameRate << std::endl;
}

bool Renderer::doesShowWireFrame() const {
    return this->showWireFrame;
}

void Renderer::setShowWireFrame(bool showWireFrame) {
    this->showWireFrame = showWireFrame;
    this->requiresRenderUpdate = true;
}

VkRenderPass Renderer::getRenderPass() const {
    return this->renderPass;
}

VkExtent2D Renderer::getSwapChainExtent() const {
    return this->swapChainExtent;
}

bool Renderer::updateRenderer() {
    if (!this->isReady()) {
        logError("Renderer has not been initialized!");
        return false;
    }

    this->stopCommandBufferQueue();
    
    this->destroySwapChainObjects();

    this->requiresRenderUpdate = false;
    
    if (!this->createSwapChain()) return false;
    if (!this->createImageViews()) return false;
    if (!this->createRenderPass()) return false;

    for (GraphicsPipeline * pipeline : this->pipelines) {
        if (pipeline != nullptr) {
            if (!pipeline->updateGraphicsPipeline()) continue;
        }
    }
    
    if (!this->createDepthResources()) return false;
    if (!this->createFramebuffers()) return false;
    
    if (!this->createCommandBuffers()) return false;
    
    return true;
}

uint32_t Renderer::getImageCount() const {
    return this->imageCount;
}

void Renderer::forceRenderUpdate() {
    this->requiresRenderUpdate = true;
}

float Renderer::getDeltaTime() {
    return this->deltaTime;
}

Renderer::~Renderer() {
    this->stopCommandBufferQueue();

    logInfo("Destroying Renderer...");
    this->destroyRendererObjects();
    
    if (this->logicalDevice != nullptr) {
        vkDestroyDevice(this->logicalDevice, nullptr);
    }
    logInfo("Destroyed Renderer");
}

