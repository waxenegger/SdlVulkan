#ifndef SRC_INCLUDES_HELPER_INCL_H_
#define SRC_INCLUDES_HELPER_INCL_H_

class Helper final {
    private:
        static std::default_random_engine default_random_engine;
        static std::uniform_real_distribution<float> distribution;

    public:
        Helper(const Helper&) = delete;
        Helper& operator=(const Helper &) = delete;
        Helper(Helper &&) = delete;
        Helper & operator=(Helper) = delete;

        static VkPresentModeKHR pickBestDeviceSwapMode(const std::vector<VkPresentModeKHR> & availableSwapModes);
        static VkImageView createImageView(const VkDevice & logicalDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t layerCount = 1, uint32_t mipLevels = 1);
        static bool createBuffer(const VkPhysicalDevice & physicalDevice, const VkDevice & logicalDevice, 
                                 const VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        static bool createImage(const VkPhysicalDevice & physicalDevice, const VkDevice & logicalDevice, 
            int32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, 
            VkImage& image, VkDeviceMemory& imageMemory, uint16_t arrayLayers = 1, uint32_t mipLevels = 1);
        static void copyBuffer(const VkDevice & logicalDevice, const VkCommandPool & commandPool, const VkQueue & graphicsQueue, 
            VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        static void copyBufferToImage(const VkDevice & logicalDevice, const VkCommandPool & commandPool, const VkQueue & graphicsQueue, VkBuffer & buffer, VkImage & image, uint32_t width, uint32_t height, uint16_t layerCount = 1);
        static bool beginCommandBuffer(VkCommandBuffer & commandBuffer);
        static VkCommandBuffer allocateCommandBuffer(const VkDevice & logicalDevice, const VkCommandPool & commandPool);
        static VkCommandBuffer allocateAndBeginCommandBuffer(const VkDevice & logicalDevice, const VkCommandPool & commandPool);
        static void endCommandBufferWithSubmit(const VkDevice & logicalDevice, const VkCommandPool & commandPool, const VkQueue & graphicsQueue, VkCommandBuffer & commandBuffer);
        static bool endCommandBuffer(VkCommandBuffer & commandBuffer);
        static void copyModelsContentIntoBuffer(void* data, ModelsContentType modelsContentType, VkDeviceSize maxSize);
        static void copyComponentsPropertiesIntoSsbo(void* data, VkDeviceSize maxSize);        
        static VkCommandPool createCommandPool(const VkDevice& logicalDevice, const uint32_t graphicsQueueIndex);
        static bool transitionImageLayout(
            const VkDevice & logicalDevice, const VkCommandPool & commandPool, const VkQueue & graphicsQueue, 
            VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint16_t layerCount = 1, uint32_t mipLevels = 1);
        static void generateMipmaps(const VkDevice & logicalDevice, const VkCommandPool & commandPool, const VkQueue & graphicsQueue, const VkImage & image, const int32_t width, const int32_t height, const uint32_t levels);
        static float getRandomFloatBetween0and1();
        static bool  checkBBoxIntersection(const BoundingBox bbox1, const BoundingBox bbox2);
};

#endif
