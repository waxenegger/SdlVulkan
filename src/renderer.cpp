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

bool Renderer::isReady() {
    return this->graphicsContext != nullptr && this->physicalDevice != nullptr && this->logicalDevice  != nullptr;
}

bool Renderer::hasAtLeastOneActivePipeline() {
    //TODO: implement
    return true;
}


bool Renderer::canRender() {
    return this->isReady() && this->hasAtLeastOneActivePipeline() && this->swapChain != nullptr && this->swapChainImages.size() == MAX_FRAMES_IN_FLIGHT && 
        this->swapChainImages.size() == this->swapChainImageViews.size() && this->imagesInFlight.size() == this->swapChainImages.size() &&
        this->imageAvailableSemaphores.size() == MAX_FRAMES_IN_FLIGHT && this->renderFinishedSemaphores.size() == MAX_FRAMES_IN_FLIGHT && this->inFlightFences.size() == MAX_FRAMES_IN_FLIGHT &&
        this->swapChainFramebuffers.size() == this->swapChainImages.size() && this->depthImages.size() == this->swapChainImages.size() && 
        this->depthImagesMemory.size() == this->swapChainImages.size() && this->depthImagesView.size() == this->swapChainImages.size() && 
        this->commandPool != nullptr && this->descriptorPool != nullptr;
}


void Renderer::addPipeline(const VkPipelineVertexInputStateCreateInfo & vertexInputCreateInfo, const VkDescriptorSetLayout & descriptorSetLayout, const VkPushConstantRange & pushConstantRange) {
    if (!this->isReady()) {
        logError("Render has not been properly initialized!");
        return;   
    }
    
    std::unique_ptr<GraphicsPipeline> pipeline = std::make_unique<GraphicsPipeline>(this->logicalDevice, this->queueIndex);
    pipeline->initGraphicsPipeline(this->renderPass, vertexInputCreateInfo, descriptorSetLayout, this->swapChainExtent, pushConstantRange, this->showWireFrame);
    
    this->pipelines.push_back(std::move(pipeline));
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

    uint32_t imageCount = MAX_FRAMES_IN_FLIGHT;
    if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
        imageCount = surfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.surface = this->graphicsContext->getVulkanSurface();
    createInfo.minImageCount = imageCount;
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
        createInfo.queueFamilyIndexCount = imageCount;
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

    ret = vkGetSwapchainImagesKHR(this->logicalDevice, this->swapChain, &imageCount, nullptr);
    if (ret != VK_SUCCESS) {
        logError("Failed to Get Swap Chain Image Count!");
        return false;
    }

    logInfo("Buffering: " + std::to_string(imageCount));
    this->swapChainImages.resize(imageCount);

    ret = vkGetSwapchainImagesKHR(this->logicalDevice, this->swapChain, &imageCount, this->swapChainImages.data());
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

    this->imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    this->renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    this->inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    this->imagesInFlight.resize(this->swapChainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

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

bool Renderer::createDescriptorPool() {
    if (!this->isReady()) {
        logError("Renderer has not been initialized!");
        return false;
    }

    std::array<VkDescriptorPoolSize, 3> poolSizes{};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(this->swapChainImages.size());
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(this->swapChainImages.size());
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_TEXTURES * this->swapChainImages.size());

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());

    VkResult ret = vkCreateDescriptorPool(this->logicalDevice, &poolInfo, nullptr, &this->descriptorPool);
    if (ret != VK_SUCCESS) {
       logError("Failed to Create Descriptor Pool!");
       return false;
    }
    
    return true;
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
    
        this->depthImagesView[i] = Helper::createImageView(this->logicalDevice, this->depthImages[i], depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
        if (this->depthImagesView[i] == nullptr) {
            logError("Faild to create Depth Image View!");
            return false;        
        }
    }
    
    return true;
}

void Renderer::initRenderer() {
    if (!this->createSwapChain()) return;
    if (!this->createImageViews()) return;
    if (!this->createRenderPass()) return;
    if (!this->createCommandPool()) return;
    if (!this->createSyncObjects()) return;
    if (!this->createDescriptorPool()) return;
    
    this->updateRenderer();
}

void Renderer::destroyRendererObjects() {
    this->destroySwapChainObjects();
    
    if (this->descriptorPool != nullptr) {
        vkDestroyDescriptorPool(this->logicalDevice, this->descriptorPool, nullptr);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (this->renderFinishedSemaphores.size() == MAX_FRAMES_IN_FLIGHT) {
            if (this->renderFinishedSemaphores[i] != nullptr) {
                vkDestroySemaphore(this->logicalDevice, this->renderFinishedSemaphores[i], nullptr);
            }
            this->renderFinishedSemaphores.clear();
        }

        if (this->imageAvailableSemaphores.size() == MAX_FRAMES_IN_FLIGHT) {
            if (this->imageAvailableSemaphores[i] != nullptr) {
                vkDestroySemaphore(this->logicalDevice, this->imageAvailableSemaphores[i], nullptr);
            }
            this->imageAvailableSemaphores.clear();
        }
        
        if (this->imagesInFlight.size() == MAX_FRAMES_IN_FLIGHT) {
            this->imagesInFlight.clear();
        }

        if (this->inFlightFences.size() == MAX_FRAMES_IN_FLIGHT) {
            if (this->inFlightFences[i] != nullptr) {
                vkDestroyFence(this->logicalDevice, this->inFlightFences[i], nullptr);
            }
            this->inFlightFences.clear();
        }
    }    
}

void Renderer::destroySwapChainObjects() {
    if (!this->isReady()) {
        logError("Renderer has not been initialized!");
        return;
    }

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

    for (auto & framebuffer : this->swapChainFramebuffers) {
        if (framebuffer != nullptr) {
            vkDestroyFramebuffer(this->logicalDevice, framebuffer, nullptr);
            framebuffer = nullptr;
        }
    }

    if (this->commandPool != nullptr) {
        vkResetCommandPool(this->logicalDevice, this->commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
    }

    // TODO: delegate
    /*
    if (this->graphicsPipeline != nullptr) {
        vkDestroyPipeline(this->device, this->graphicsPipeline, nullptr);
        this->graphicsPipeline = nullptr;
    }
    if (this->graphicsPipelineLayout != nullptr) {
        vkDestroyPipelineLayout(this->device, this->graphicsPipelineLayout, nullptr);
        this->graphicsPipelineLayout = nullptr;
    }
    */
    
    if (this->renderPass != nullptr) {
        vkDestroyRenderPass(this->logicalDevice, this->renderPass, nullptr);
        this->renderPass = nullptr;
    }
    
    for (auto & imageView : this->swapChainImageViews) {
        if (imageView == nullptr) {
            vkDestroyImageView(this->logicalDevice, imageView, nullptr);
            imageView = nullptr;
        }
    }

    if (this->swapChain != nullptr) {
        vkDestroySwapchainKHR(this->logicalDevice, this->swapChain, nullptr);
        this->swapChain = nullptr;
    }

}

void Renderer::updateRenderer() {
    if (!this->isReady()) {
        logError("Renderer has not been initialized!");
        return;
    }

    this->destroySwapChainObjects();
    
    if (!this->createSwapChain()) return;
    if (!this->createImageViews()) return;
    if (!this->createRenderPass()) return;

    //TODO: delegate
    //if (!this->createGraphicsPipeline()) return;
    
    if (!this->createDepthResources()) return;
    if (!this->createFramebuffers()) return;

}


Renderer::~Renderer() {
    this->destroyRendererObjects();
    
    if (this->logicalDevice != nullptr) {
        vkDestroyDevice(this->logicalDevice, nullptr);
        this->logicalDevice = nullptr;
    }
}

