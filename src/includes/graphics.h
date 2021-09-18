#ifndef SRC_INCLUDES_GRAPHICS_INCL_H_
#define SRC_INCLUDES_GRAPHICS_INCL_H_

#include "shared.h"
#include "models.h"
#include "threading.h"

constexpr uint32_t VULKAN_VERSION = VK_MAKE_VERSION(1,1,0);

static constexpr int MAX_TEXTURES = 50;
static constexpr int MAX_FRAMES_IN_FLIGHT = 3;

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
        
        VkShaderStageFlagBits getShaderType() const;
        VkShaderModule getShaderModule() const;

        Shader(const VkDevice device, const std::string & filename, const VkShaderStageFlagBits & shaderType);
        ~Shader();
        
        bool isValid() const;
};

class GraphicsContext final {
    private:
        SDL_Window * sdlWindow = nullptr;
        VkInstance vulkanInstance = nullptr;
        VkSurfaceKHR vulkanSurface = nullptr;
        
        std::vector<const char *> vulkanExtensions;
        std::vector<const char *> vulkanLayers = {
           "VK_LAYER_KHRONOS_validation"
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
        
        void createVulkanInstance(const std::string & appName, const uint32_t version = VULKAN_VERSION);

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
        
        VkSurfaceKHR getVulkanSurface() const;
        
        VkExtent2D getSwapChainExtent(VkSurfaceCapabilitiesKHR & surfaceCapabilities) const;
        bool getSurfaceCapabilities(const VkPhysicalDevice & physicalDevice, VkSurfaceCapabilitiesKHR & surfaceCapabilities) const;
        std::vector<VkPresentModeKHR> queryDeviceSwapModes(const VkPhysicalDevice & physicalDevice) const;
        
        GraphicsContext();
        ~GraphicsContext();
};

class GraphicsPipeline final {
    private:
        std::map<std::string, const Shader *> shaders;
        const VkDevice & device = nullptr;
        
        VkDescriptorSetLayout descriptorSetLayout;
                
        VkDescriptorPool descriptorPool = nullptr;
        std::vector<VkDescriptorSet> descriptorSets;

        std::vector<VkBuffer> uniformBuffers;
        std::vector<VkDeviceMemory> uniformBuffersMemory;

        VkSampler textureSampler = nullptr;
        
        VkBuffer ssboBuffer = nullptr;
        VkDeviceMemory ssboBufferMemory = nullptr;
        
        VkPipelineLayout layout = nullptr;
        VkPipeline pipeline = nullptr;

        bool createDescriptorSetLayout();
        bool createDescriptorPool(const size_t size = MAX_FRAMES_IN_FLIGHT);
        bool createDescriptorSets(const size_t size = MAX_FRAMES_IN_FLIGHT);
        bool createSsboBufferFromModel(const VkPhysicalDevice & physicalDevice, const VkCommandPool & commandpool, const VkQueue & graphicsQueue, VkDeviceSize bufferSize, bool makeHostWritable = false);
        bool createTextureSampler(const VkPhysicalDevice & physicalDevice, VkSampler & sampler, VkSamplerAddressMode addressMode);
        
        void destroyPipelineObjects();

    public:
        GraphicsPipeline(const GraphicsPipeline&) = delete;
        GraphicsPipeline& operator=(const GraphicsPipeline &) = delete;
        GraphicsPipeline(GraphicsPipeline &&) = delete;
        GraphicsPipeline & operator=(GraphicsPipeline) = delete;

        void addShader(const std::string & file, const VkShaderStageFlagBits & shaderType);
        
        std::vector<VkPipelineShaderStageCreateInfo> getShaderStageCreateInfos();
        

        bool createGraphicsPipeline(
            const size_t size, const VkPhysicalDevice & physicalDevice, const VkRenderPass & renderPass, const VkCommandPool & commandpool, const VkQueue & graphicsQueue, 
            const VkExtent2D & swapChainExtent, const VkPushConstantRange & pushConstantRange, bool showWireFrame = false);
        bool updateGraphicsPipeline(const VkRenderPass & renderPass, const VkExtent2D & swapChainExtent, const VkPushConstantRange & pushConstantRange, bool showWireFrame = false);
        void updateUniformBuffers(const ModelUniforms & modelUniforms, const uint32_t & currentImage);
        
        void draw(const VkCommandBuffer & commandBuffer);
        
        GraphicsPipeline(const VkDevice & device);
        ~GraphicsPipeline();
};

class Renderer final {
    private:
        const GraphicsContext * graphicsContext = nullptr;
        const VkPhysicalDevice physicalDevice = nullptr;
        VkDevice logicalDevice = nullptr;

        VkCommandPool commandPool = nullptr;
        CommandBufferQueue workerQueue;

        std::vector<VkCommandBuffer> commandBuffers;
        
        const int queueIndex = -1;
        
        uint32_t graphicsQueueIndex = -1;
        VkQueue graphicsQueue = nullptr;
        uint32_t presentQueueIndex = -1;
        VkQueue presentQueue = nullptr;

        std::vector<GraphicsPipeline *> pipelines;
        
        uint16_t frameCount = 0;
        size_t currentFrame = 0;
        
        bool requiresRenderUpdate = false;
        bool showWireFrame = false;
         
        VkRenderPass renderPass = nullptr;
        VkExtent2D swapChainExtent;
        
        VkSwapchainKHR swapChain = nullptr;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;
        std::vector<VkFramebuffer> swapChainFramebuffers;
        std::vector<VkImage> depthImages;
        std::vector<VkDeviceMemory> depthImagesMemory;
        std::vector<VkImageView> depthImagesView;

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;

        bool createRenderPass();
        bool createSwapChain();
        bool createImageViews();
        bool createFramebuffers();
        bool createDepthResources();
        
        bool createDescriptorSets();

        bool createCommandPool();
        bool createSyncObjects();
        
        bool createCommandBuffers();
        void startCommandBufferQueue();
        void stopCommandBufferQueue();
        void destroyCommandBuffer(VkCommandBuffer commandBuffer);
        VkCommandBuffer createCommandBuffer(uint16_t commandBufferIndex);
        
        void updateUniformBuffer(uint32_t currentImage);
        
        void destroySwapChainObjects();        
        void destroyRendererObjects();

    public:
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer &) = delete;
        Renderer(GraphicsPipeline &&) = delete;
        Renderer & operator=(Renderer) = delete;
        Renderer(const GraphicsContext * graphicsContext, const VkPhysicalDevice & physicalDevice, const int & queueIndex);
        
        void addPipeline(GraphicsPipeline * pipeline);
        
        bool isReady();
        bool hasAtLeastOneActivePipeline();
        bool canRender();
                
        VkDevice getLogicalDevice();
        VkPhysicalDevice getPhysicalDevice();
        
        void initRenderer();
        bool updateRenderer();
        
        bool doesShowWireFrame();
        void setShowWireFrame(bool & showWireFrame);

        VkRenderPass getRenderPass();
        VkExtent2D getSwapChainExtent();
        
        VkCommandPool getCommandPool();
        VkQueue getGraphicsQueue();

        void drawFrame();
        
        ~Renderer();
};


class Helper final {
    public:
        Helper(const Helper&) = delete;
        Helper& operator=(const Helper &) = delete;
        Helper(Helper &&) = delete;
        Helper & operator=(Helper) = delete;

        static VkPresentModeKHR pickBestDeviceSwapMode(const std::vector<VkPresentModeKHR> & availableSwapModes);
        static VkImageView createImageView(const VkDevice & logicalDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t layerCount = 1);
        static bool createBuffer(const VkPhysicalDevice & physicalDevice, const VkDevice & logicalDevice, 
                                 const VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        static bool createImage(const VkPhysicalDevice & physicalDevice, const VkDevice & logicalDevice, 
            int32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, 
            VkImage& image, VkDeviceMemory& imageMemory, uint16_t arrayLayers = 1);
        static void copyBuffer(const VkDevice & logicalDevice, const VkCommandPool & commandPool, const VkQueue & graphicsQueue, 
            VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        static void copyBufferToImage(const VkDevice & logicalDevice, const VkCommandPool & commandPool, const VkQueue & graphicsQueue, VkBuffer & buffer, VkImage & image, uint32_t width, uint32_t height, uint16_t layerCount = 1);
        static VkCommandBuffer beginSingleTimeCommands(const VkDevice & logicalDevice, const VkCommandPool & commandPool);
        static void endSingleTimeCommands(const VkDevice & logicalDevice, const VkCommandPool & commandPool, const VkQueue & graphicsQueue, VkCommandBuffer & commandBuffer);
        static void copyModelsContentIntoBuffer(void* data, ModelsContentType modelsContentType, VkDeviceSize maxSize);
};

class Engine final {
    private:
        GraphicsContext * graphics = new GraphicsContext();
        Models * models = Models::INSTANCE();
        Renderer * renderer = nullptr;

        void createRenderer();
        void createModelPipeline();
        
    public:
        Engine(const Helper&) = delete;
        Engine& operator=(const Engine &) = delete;
        Engine(Engine &&) = delete;
        Engine & operator=(Engine) = delete;
        
        bool isReady();
        
        void init();
        void loop();

        void loadModels();
        
        Engine(const std::string & appName, const uint32_t version = VULKAN_VERSION);
        ~Engine();
};


#endif
