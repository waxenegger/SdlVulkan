#include "includes/graphics.h"

bool Helper::createImage(
    const VkPhysicalDevice & physicalDevice, const VkDevice & logicalDevice, 
    int32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, 
    VkImage& image, VkDeviceMemory& imageMemory, uint16_t arrayLayers) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = arrayLayers;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (arrayLayers > 1) {
            imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;    
        }
        
        VkResult ret = vkCreateImage(logicalDevice, &imageInfo, nullptr, &image);
        if (ret != VK_SUCCESS) {
            logError("Failed to Create Image");
            return false;
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

        uint32_t memoryTypeIndex;
        if (!GraphicsContext::findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties, memoryTypeIndex)) {
            logError("Failed to get Memory Type Requested");
            return false;            
        }

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = memoryTypeIndex;
        
        ret = vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &imageMemory);
        if (ret != VK_SUCCESS) {
            logError("Failed to Allocate Image Memory");
            return false;
        }
        
        vkBindImageMemory(logicalDevice, image, imageMemory, 0);
        
        return true;
}

VkImageView Helper::createImageView(const VkDevice & logicalDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t layerCount) {
    if (logicalDevice == nullptr) {
        logError("Renderer has not been initialized!");
        return nullptr;
    }

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = layerCount > 1 ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = layerCount;

    VkImageView imageView = nullptr;
    VkResult ret = vkCreateImageView(logicalDevice, &viewInfo, nullptr, &imageView);
    if (ret != VK_SUCCESS) {
        logError("Failed to Create Image View!");
        return nullptr;
    }

    return imageView;
}

VkPresentModeKHR Helper::pickBestDeviceSwapMode(const std::vector<VkPresentModeKHR> & availableSwapModes) {
    for (auto & swapMode : availableSwapModes) {
        if (swapMode == VK_PRESENT_MODE_MAILBOX_KHR) return swapMode;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

bool Helper::createBuffer(
    const VkPhysicalDevice & physicalDevice, const VkDevice & logicalDevice, const VkDeviceSize size, 
    VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult ret = vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer);
    if (ret != VK_SUCCESS) {
        logError("Failed to get Create Buffer!");
        return false;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(logicalDevice, buffer, &memRequirements);

    uint32_t memoryTypeIndex;
    if (!GraphicsContext::findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties,memoryTypeIndex)) {
        logError("Failed to get Memory Type Requested!");
        return false;
    }

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;


    ret = vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &bufferMemory);
    if (ret != VK_SUCCESS) {
        logError("Failed to Allocate Memory for Buffer!");
        return false;
    }

    vkBindBufferMemory(logicalDevice, buffer, bufferMemory, 0);

    return true;
}

VkCommandBuffer Helper::beginCommandBuffer(const VkDevice & logicalDevice, const VkCommandPool & commandPool, const VkCommandBufferInheritanceInfo * cmdBufferInherit) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = cmdBufferInherit == nullptr ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = nullptr;

    VkResult ret = vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);
    if (ret != VK_SUCCESS) {
        logError("Failed to Allocate Command Buffer!");
        return nullptr;
    }


    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = cmdBufferInherit == nullptr ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = cmdBufferInherit;

    ret = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (ret != VK_SUCCESS) {
        logError("Failed to Begin Command Buffer!");
        return nullptr;
    }

    return commandBuffer;
}

void Helper::endCommandBufferWithSubmit(const VkDevice & logicalDevice, const VkCommandPool & commandPool, const VkQueue & graphicsQueue, VkCommandBuffer & commandBuffer) {
    VkResult ret = vkEndCommandBuffer(commandBuffer);
    if (ret != VK_SUCCESS) {
        logError("Failed to End Command Buffer!");
        return;
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    // TODO: revisit
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}

bool Helper::endCommandBuffer(VkCommandBuffer & commandBuffer) {
    VkResult ret = vkEndCommandBuffer(commandBuffer);
    if (ret != VK_SUCCESS) {
        logError("Failed to End Command Buffer!");
        return false;
    }
    
    return true;
}

void Helper::copyBuffer(
    const VkDevice & logicalDevice, const VkCommandPool & commandPool, const VkQueue & graphicsQueue, 
    VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    
    VkCommandBuffer commandBuffer = Helper::beginCommandBuffer(logicalDevice, commandPool);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    Helper::endCommandBufferWithSubmit(logicalDevice, commandPool, graphicsQueue, commandBuffer);
}


void Helper::copyBufferToImage(
    const VkDevice & logicalDevice, const VkCommandPool & commandPool, const VkQueue & graphicsQueue, 
    VkBuffer & buffer, VkImage & image, uint32_t width, uint32_t height, uint16_t layerCount) {
    
    
    VkCommandBuffer commandBuffer = Helper::beginCommandBuffer(logicalDevice, commandPool);
    if (commandBuffer == nullptr) return;

    std::vector<VkBufferImageCopy> regions;
    VkBufferImageCopy region;
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layerCount;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = { width, height, 1};
    regions.push_back(region);

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regions.size(), regions.data());

    Helper::endCommandBufferWithSubmit(logicalDevice, commandPool, graphicsQueue, commandBuffer);    
}

void Helper::copyModelsContentIntoBuffer(void* data, ModelsContentType modelsContentType, VkDeviceSize maxSize) {
    VkDeviceSize overallSize = 0;
    
    auto & allModels = Models::INSTANCE()->getModels();
    
    for (auto & model : allModels) {
        for (Mesh & mesh : model->getMeshes()) {            
            VkDeviceSize dataSize = 0;
            switch(modelsContentType) {
                case INDEX:
                    dataSize = mesh.getIndices().size() * sizeof(uint32_t);
                    if (overallSize + dataSize <= maxSize) {
                        memcpy(static_cast<char *>(data) + overallSize, mesh.getIndices().data(), dataSize);
                        overallSize += dataSize;
                    }
                    break;
                case VERTEX:
                    dataSize = mesh.getVertices().size() * sizeof(class ModelVertex);
                    if (overallSize + dataSize <= maxSize) {
                        memcpy(static_cast<char *>(data)+overallSize, mesh.getVertices().data(), dataSize);
                        overallSize += dataSize;
                    }
                    break;
                case SSBO:
                    TextureInformation textureInfo = mesh.getTextureInformation();
                    MaterialInformation materialInfo = mesh.getMaterialInformation();
                    MeshProperties modelProps = { 
                        textureInfo.ambientTexture,
                        textureInfo.diffuseTexture,
                        textureInfo.specularTexture,
                        textureInfo.normalTexture,
                        materialInfo.ambientColor,
                        materialInfo.emissiveFactor,
                        materialInfo.diffuseColor,
                        materialInfo.opacity,
                        materialInfo.specularColor,
                        materialInfo.shininess
                    };
                    
                    dataSize = sizeof(struct MeshProperties);             
                    if (overallSize + dataSize <= maxSize) {
                        memcpy(static_cast<char *>(data)+overallSize, &modelProps, dataSize);
                        overallSize += dataSize;
                    }
                    break;
            }
        }
    }
}

bool Helper::transitionImageLayout(
    const VkDevice & logicalDevice, const VkCommandPool & commandPool, const VkQueue & graphicsQueue, 
    VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint16_t layerCount) {
    
    VkCommandBuffer commandBuffer = Helper::beginCommandBuffer(logicalDevice, commandPool);
    if (commandBuffer == nullptr) return false;

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layerCount;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;        
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == newLayout) {
        return true;
    } else
    {
        std::cerr << "Unsupported Layout Transition" << std::endl;
        return false;
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    Helper::endCommandBufferWithSubmit(logicalDevice, commandPool, graphicsQueue, commandBuffer);
    
    return true;
}

VkCommandPool Helper::createCommandPool(const VkDevice & logicalDevice, const u_int32_t graphicsQueueIndex) {
    if (logicalDevice == nullptr) {
        logError("Logical Device is null!");
        return nullptr;
    }
    
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = graphicsQueueIndex;
    
    VkCommandPool pool = nullptr;

    VkResult ret = vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &pool);
    if (ret != VK_SUCCESS) {
        logError("Failed to Create Command Pool!");
        return nullptr;
    }

    return pool;
}

