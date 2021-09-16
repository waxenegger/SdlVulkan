#ifndef SRC_INCLUDES_GRAPHICS_INCL_H_
#define SRC_INCLUDES_GRAPHICS_INCL_H_

#include "shared.h"
#include "models.h"

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
        std::map<std::string, std::unique_ptr<Shader>> shaders;
        const VkDevice & device = nullptr;
        
        VkPipelineLayout layout = nullptr;
        VkPipeline pipeline = nullptr;
        
        void destroyPipelineObjects();

    public:
        GraphicsPipeline(const GraphicsPipeline&) = delete;
        GraphicsPipeline& operator=(const GraphicsPipeline &) = delete;
        GraphicsPipeline(GraphicsPipeline &&) = delete;
        GraphicsPipeline & operator=(GraphicsPipeline) = delete;

        void addShader(const std::string & file, const VkShaderStageFlagBits & shaderType);
        
        std::vector<VkPipelineShaderStageCreateInfo> getShaderStageCreateInfos();
        
        void initGraphicsPipeline(const VkRenderPass & renderPass,
            const VkPipelineVertexInputStateCreateInfo & vertexInputCreateInfo, const VkDescriptorSetLayout & descriptorSetLayout, 
            const VkExtent2D & swapChainExtent, const VkPushConstantRange & pushConstantRange, bool showWireFrame = false);
        
        GraphicsPipeline(const VkDevice & device, const int & queueIndex);
        ~GraphicsPipeline();
};

class Renderer final {
    private:
        const GraphicsContext * graphicsContext = nullptr;
        const VkPhysicalDevice physicalDevice = nullptr;
        VkDevice logicalDevice = nullptr;

        VkCommandPool commandPool = nullptr;
        VkDescriptorPool descriptorPool = nullptr;

        const int queueIndex = -1;
        
        uint32_t graphicsQueueIndex = -1;
        VkQueue graphicsQueue = nullptr;
        uint32_t presentQueueIndex = -1;
        VkQueue presentQueue = nullptr;

        std::vector<std::unique_ptr<GraphicsPipeline>> pipelines;
        
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

        bool createCommandPool();
        bool createSyncObjects();
        bool createDescriptorPool();
    
        void destroySwapChainObjects();        
        void destroyRendererObjects();

    public:
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer &) = delete;
        Renderer(GraphicsPipeline &&) = delete;
        Renderer & operator=(Renderer) = delete;
        Renderer(const GraphicsContext * graphicsContext, const VkPhysicalDevice & physicalDevice, const int & queueIndex);
        
        void addPipeline(const VkPipelineVertexInputStateCreateInfo & vertexInputCreateInfo, const VkDescriptorSetLayout & descriptorSetLayout, const VkPushConstantRange & pushConstantRange);
        
        bool isReady();
        bool hasAtLeastOneActivePipeline();
        bool canRender();

                
        VkDevice getLogicalDevice();
        
        void initRenderer();
        bool updateRenderer();
        
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
        static VkImageView createImageView(const VkDevice logicalDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t layerCount = 1);
        static bool createImage(const VkPhysicalDevice & physicalDevice, const VkDevice & logicalDevice, 
            int32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, 
            VkImage& image, VkDeviceMemory& imageMemory, uint16_t arrayLayers = 1);
};

class Engine final {
    private:
        GraphicsContext * graphics = new GraphicsContext();
        Models * models = new Models();
        Renderer * renderer = nullptr;


    public:
        Engine(const Helper&) = delete;
        Engine& operator=(const Engine &) = delete;
        Engine(Engine &&) = delete;
        Engine & operator=(Engine) = delete;
        
        bool isReady();
        
        void createDefaultRenderer();
        void loop();

        void loadModels();
        
        Engine(const std::string & appName, const uint32_t version = VULKAN_VERSION);
        ~Engine();
};


#endif
