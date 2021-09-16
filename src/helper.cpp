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

VkImageView Helper::createImageView(const VkDevice logicalDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t layerCount) {
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

