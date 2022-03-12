#ifndef SRC_INCLUDES_GRAPHICS_INCL_H_
#define SRC_INCLUDES_GRAPHICS_INCL_H_

#include "shared.h"
#include "threading.h"
#include "models.h"
#include "components.h"
#include "world.h"

constexpr uint32_t VULKAN_VERSION = VK_MAKE_VERSION(1,0,0);

const VkSurfaceFormatKHR SWAP_CHAIN_IMAGE_FORMAT = {
    #ifndef __ANDROID__
        VK_FORMAT_B8G8R8A8_SRGB,
    #endif
    #ifdef __ANDROID__
        VK_FORMAT_R8G8B8A8_SRGB,
    #endif
        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
};

class Shader final {
    private:
        std::string filename;
        VkShaderStageFlagBits shaderType;
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
        std::string getFileName() const;

        Shader(const VkDevice device, const std::string & filename, const VkShaderStageFlagBits shaderType);
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
           //"VK_LAYER_KHRONOS_validation"
           //"VK_LAYER_ADRENO_debug"
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
        SDL_Window * getSdlWindow() const;
        VkInstance getVulkanInstance() const;
        
        VkExtent2D getSwapChainExtent(VkSurfaceCapabilitiesKHR & surfaceCapabilities) const;
        bool getSurfaceCapabilities(const VkPhysicalDevice & physicalDevice, VkSurfaceCapabilitiesKHR & surfaceCapabilities) const;
        std::vector<VkPresentModeKHR> queryDeviceSwapModes(const VkPhysicalDevice & physicalDevice) const;
        
        GraphicsContext();
        ~GraphicsContext();
};

class Renderer;

class GraphicsPipeline {
    protected:
        std::map<std::string, const Shader *> shaders;
        const Renderer * renderer = nullptr;
        
        bool enabled = true;
        bool showBoundingBoxes = false;
        
        VkDescriptorSetLayout descriptorSetLayout = nullptr;
                
        VkDescriptorPool descriptorPool = nullptr;
        std::vector<VkDescriptorSet> descriptorSets;
        
        VkPushConstantRange pushConstantRange;

        VkSampler textureSampler = nullptr;
        
        VkBuffer vertexBuffer = nullptr;
        VkDeviceMemory vertexBufferMemory = nullptr;
        VkBuffer indexBuffer = nullptr;
        VkDeviceMemory indexBufferMemory = nullptr;

        VkBuffer ssboBuffer = nullptr;
        VkDeviceMemory ssboBufferMemory = nullptr;
        
        VkPipelineLayout layout = nullptr;
        VkPipeline pipeline = nullptr;

        virtual bool createDescriptorSetLayout() = 0;
        virtual bool createDescriptorPool() = 0;
        virtual bool createDescriptorSets() = 0;

    public:
        GraphicsPipeline(const GraphicsPipeline&) = delete;
        GraphicsPipeline& operator=(const GraphicsPipeline &) = delete;
        GraphicsPipeline(GraphicsPipeline &&) = delete;

        void addShader(const std::string & file, const VkShaderStageFlagBits & shaderType);
        
        std::vector<VkPipelineShaderStageCreateInfo> getShaderStageCreateInfos();
        
        bool isReady() const;
        virtual bool canRender() const;

        virtual bool createGraphicsPipeline(const VkPushConstantRange & pushConstantRange) = 0;
        virtual bool updateGraphicsPipeline() = 0;

        bool createTextureSampler(VkSamplerAddressMode addressMode);
        void destroyPipelineObjects();
        
        virtual void draw(const VkCommandBuffer & commandBuffer, const uint16_t commandBufferIndex) = 0;
        virtual void update() = 0;
        
        bool isEnabled();
        void setEnabled(const bool flag);
        
        bool isShowingBoundingBoxes();
        void setShowBoundingBoxes(const bool flag);
        
        GraphicsPipeline(const Renderer * renderer);
        virtual ~GraphicsPipeline();
};

class ModelsPipeline : public GraphicsPipeline {
    private:
        bool createSsboBufferFromModel(VkDeviceSize bufferSize, bool makeHostWritable = false);
        bool createBuffersFromModel();
        void prepareModelTextures();

        bool createDescriptorSetLayout();
        bool createDescriptorPool();
        bool createDescriptorSets();
        
        void updateSsboBuffersComponents();
    public:
        ModelsPipeline(const Renderer * renderer);
        ModelsPipeline & operator=(ModelsPipeline) = delete;

        bool createSsboBuffersFromComponents();
        bool createGraphicsPipeline(const VkPushConstantRange & pushConstantRange);
        bool updateGraphicsPipeline();
        
        void draw(const VkCommandBuffer & commandBuffer, const uint16_t commandBufferIndex);
        void update();
};

class SkyboxPipeline : public GraphicsPipeline {
    private:
        std::array<std::string, 6> skyboxCubeImageLocations = { "right.png", "left.png", "top.png", "bottom.png", "front.png", "back.png" };

        VkImageView imageView = nullptr;
        VkImage cubeImage = nullptr;
        VkDeviceMemory cubeImageMemory = nullptr;

        bool createSkybox();
        
        bool createDescriptorSetLayout();
        bool createDescriptorPool();
        bool createDescriptorSets();
        
    public:
        SkyboxPipeline(const Renderer * renderer);
        SkyboxPipeline & operator=(SkyboxPipeline) = delete;

        bool createGraphicsPipeline(const VkPushConstantRange & pushConstantRange);
        bool updateGraphicsPipeline();
        
        void draw(const VkCommandBuffer & commandBuffer, const uint16_t commandBufferIndex);
        void update();
        
        ~SkyboxPipeline();
};

class ImGuiPipeline : public GraphicsPipeline {
    private:
        bool createDescriptorPool();
        bool createDescriptorSetLayout();
        bool createDescriptorSets();
        
    public:
        ImGuiPipeline(const Renderer * renderer);
        ImGuiPipeline & operator=(ImGuiPipeline) = delete;

        bool createGraphicsPipeline(const VkPushConstantRange & pushConstantRange);
        bool updateGraphicsPipeline();
        bool canRender() const;
        
        void draw(const VkCommandBuffer & commandBuffer, const uint16_t commandBufferIndex);
        void update();
        
        ~ImGuiPipeline();
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
        uint32_t imageCount = MAX_BUFFERING;
        
        uint32_t graphicsQueueIndex = -1;
        VkQueue graphicsQueue = nullptr;
        uint32_t presentQueueIndex = -1;
        VkQueue presentQueue = nullptr;

        std::vector<VkBuffer> uniformBuffers;
        std::vector<VkDeviceMemory> uniformBuffersMemory;
        
        std::vector<GraphicsPipeline *> pipelines;
        
        std::vector<float> deltaTimes;
        float lastDeltaTime = DELTA_TIME_60FPS;
        std::chrono::high_resolution_clock::time_point lastFrameRateUpdate;        
        uint16_t frameRate = FRAME_RATE_60;
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

        bool createRenderPass();
        bool createSwapChain();
        bool createImageViews();
        bool createFramebuffers();
        bool createDepthResources();
        
        bool createDescriptorSets();

        bool createCommandPool();
        bool createSyncObjects();
        
        bool createCommandBuffers();
        void destroyCommandBuffer(VkCommandBuffer commandBuffer);
        VkCommandBuffer createCommandBuffer(uint16_t commandBufferIndex);
        
        bool createUniformBuffers();
        void updateUniformBuffer(uint32_t currentImage);
        void updateUniformBuffers(const ModelUniforms & modelUniforms, const uint32_t & currentImage);
        
        void destroySwapChainObjects();        
        void destroyRendererObjects();
    public:
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer &) = delete;
        Renderer(GraphicsPipeline &&) = delete;
        Renderer & operator=(Renderer) = delete;
        Renderer(const GraphicsContext * graphicsContext, const VkPhysicalDevice & physicalDevice, const int & queueIndex);
        
        uint8_t addPipeline(GraphicsPipeline * pipeline);
        void enablePipeline(const uint8_t index, const bool flag = true);

        void startCommandBufferQueue();
        void stopCommandBufferQueue();
        
        bool isReady() const;
        bool hasAtLeastOneActivePipeline() const;
        virtual bool canRender() const;
                
        VkDevice getLogicalDevice() const;
        VkPhysicalDevice getPhysicalDevice() const;
        uint32_t getImageCount() const;
        
        void initRenderer();
        bool updateRenderer();
        
        void forceRenderUpdate();
        bool doesShowWireFrame() const;
        void setShowWireFrame(bool showWireFrame);

        VkRenderPass getRenderPass() const;
        VkExtent2D getSwapChainExtent() const;
        
        VkCommandPool getCommandPool() const;
        VkQueue getGraphicsQueue() const;
        uint32_t getGraphicsQueueIndex() const;
        
        const VkBuffer getUniformBuffer(uint8_t index) const;

        GraphicsPipeline * getPipeline(uint8_t index);
        
        const GraphicsContext * getGraphicsContext() const;
            
        uint16_t getFrameRate() const;
        void addDeltaTime(const std::chrono::high_resolution_clock::time_point now, const float deltaTime);
        float getDeltaFactor();
        
        void drawFrame();
        
        ~Renderer();
};

class Engine final {
    private:
        static std::filesystem::path base;
        GraphicsContext * graphics = new GraphicsContext();
        Models * models = Models::INSTANCE();
        Components * components = Components::INSTANCE();
        Camera * camera = Camera::INSTANCE();
        Renderer * renderer = nullptr;
        
        bool showSkybox = false;
        bool showComponents = true;
        bool showGuiOverlay = false;
        bool showBoundingBoxes = false;
        
        int skyboxPipelineIndex = -1;
        int modelPipelineIndex = -1;
        int guiPipelineIndex = -1;
        
        bool quit = false;

        void createRenderer();
        void createModelPipeline();
        void updateModelPipeline();
        void createSkyboxPipeline();
        void createImGuiPipeline();
        
        void inputLoop();
        
    public:
        Engine(const Engine&) = delete;
        Engine& operator=(const Engine &) = delete;
        Engine(Engine &&) = delete;
        Engine & operator=(Engine) = delete;
        
        bool isReady();
        
        void init();
        void loop();
        
        void startInputCapture();

        void preloadModels();
        void updateModels(const std::string id, const std::filesystem::path file);
        
        Engine(const std::string & appName, const std::string root = "", const uint32_t version = VULKAN_VERSION);
        ~Engine();
        
        static std::filesystem::path getAppPath(APP_PATHS appPath);
        
        void setShowSkybox(const bool flag);
        void setShowComponents(const bool flag);
        void setShowGuiOverlay(const bool flag);
        void setShowBoundingBoxes(const bool flag);
        
        float getDeltaFactor();
};

#endif
