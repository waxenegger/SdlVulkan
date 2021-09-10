#ifndef SRC_INCLUDES_GRAPHICS_INCL_H_
#define SRC_INCLUDES_GRAPHICS_INCL_H_

#include "shared.h"

const VkSurfaceFormatKHR SWAP_CHAIN_IMAGE_FORMAT = {
        VK_FORMAT_B8G8R8A8_SRGB,
        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
};


class Shader final {
    private:
        std::string filename;
        const VkShaderStageFlagBits & shaderType;
        VkShaderModule shaderModule = nullptr;
        VkDevice device;
        
        bool readFile(const std::string & filename, std::vector<char> & buffer);
        VkShaderModule createShaderModule(const std::vector<char> & code);
    public:
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader &) = delete;
        Shader(Shader &&) = delete;
        Shader & operator=(Shader) = delete;
        
        VkShaderStageFlagBits getShaderType();
        VkShaderModule getShaderModule();

        Shader(const VkDevice device, const std::string & filename, const VkShaderStageFlagBits & shaderType);
        ~Shader();
        
        bool isValid();
};

class GraphicsContext final {
    private:
        SDL_Window * sdlWindow = nullptr;
        VkInstance vulkanInstance = nullptr;
        VkSurfaceKHR vulkanSurface = nullptr;
        
        std::vector<const char *> vulkanExtensions;
        std::vector<const char *> vulkanLayers = {
           //"VK_LAYER_KHRONOS_validation"
        };
        
        std::vector<VkPhysicalDevice> physicalDevices;
                
        bool queryVulkanInstanceExtensions();
        const std::vector<VkExtensionProperties> queryDeviceExtensions(const VkPhysicalDevice & device);
        void queryPhysicalDevices();
        
        const std::vector<VkSurfaceFormatKHR> queryPhysicalDeviceSurfaceFormats(const VkPhysicalDevice & device);
        bool doesPhysicalDeviceSupportExtension(const VkPhysicalDevice & device, const std::string extension);
        bool isPhysicalDeviceSurfaceFormatsSupported(const VkPhysicalDevice & device, const VkSurfaceFormatKHR & format);
        const std::vector<VkQueueFamilyProperties> getPhysicalDeviceQueueFamilyProperties(const VkPhysicalDevice & device);
        const std::tuple<int,int> ratePhysicalDevice(const VkPhysicalDevice & device);
        
        void createVulkanInstance(const std::string & appName, const uint32_t version);

        void quitSdl();
        void quitVulkan();
        void quitGraphics();

    public:
        GraphicsContext(const GraphicsContext&) = delete;
        GraphicsContext& operator=(const GraphicsContext &) = delete;
        GraphicsContext(GraphicsContext &&) = delete;
        GraphicsContext & operator=(GraphicsContext) = delete;
        
        bool isSdlActive() const;
        bool isVulkanActive() const;
        bool isGraphicsActive() const;
        
        void initSdl(const std::string & appName);
        void initVulkan(const std::string & appName, const uint32_t version);
        void initGraphics(const std::string & appName, const uint32_t version);

        const std::tuple<VkPhysicalDevice, int> pickBestPhysicalDeviceAndQueueIndex();
        
        void listVulkanExtensions();
        void listLayerNames();
        void listPhysicalDevices();
        
        static bool findDepthFormat(const VkPhysicalDevice & device, VkFormat & supportedFormat);
        static bool findSupportedFormat(const VkPhysicalDevice & device,
            const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkFormat & supportedFormat);        
        static bool findMemoryType(const VkPhysicalDevice & device, uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t & memoryType);
        
        GraphicsContext();
        ~GraphicsContext();
};

class GraphicsPipeline final {
    private:
        std::map<std::string, std::unique_ptr<Shader>> shaders;
        VkDevice device = nullptr;
        
        VkPipelineLayout layout = nullptr;
        VkPipeline pipeline = nullptr;
        VkRenderPass renderPass = nullptr;
        
        uint32_t graphicsQueueIndex = -1;
        VkQueue graphicsQueue = nullptr;
        uint32_t presentQueueIndex = -1;
        VkQueue presentQueue = nullptr;
        
        VkFormat depthFormat;
        
        void createRenderPass();
        void destroyPipelineObjects();

    public:
        GraphicsPipeline(const GraphicsPipeline&) = delete;
        GraphicsPipeline& operator=(const GraphicsPipeline &) = delete;
        GraphicsPipeline(GraphicsPipeline &&) = delete;
        GraphicsPipeline & operator=(GraphicsPipeline) = delete;

        void addShader(const std::string & file, const VkShaderStageFlagBits & shaderType);
        
        std::vector<VkPipelineShaderStageCreateInfo> getShaderStageCreateInfos();
        
        void createGraphicsPipeline(
            const VkPipelineVertexInputStateCreateInfo & vertexInputCreateInfo, const VkDescriptorSetLayout & descriptorSetLayout, 
            const VkExtent2D & swapChainExtent, const VkPushConstantRange & pushConstantRange, bool showWireFrame = false);
        
        GraphicsPipeline(const VkPhysicalDevice physicalDevice, const int queueIndex);
        ~GraphicsPipeline();
};

#endif
