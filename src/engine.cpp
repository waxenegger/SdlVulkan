#include "includes/graphics.h"

Engine::Engine(const std::string & appName, const std::string root, const uint32_t version) {
    logInfo("Creating Graphics Context...");
    this->graphics->initGraphics("Test App", VULKAN_VERSION);

    if(!this->graphics->isGraphicsActive()) {
        logError("Could not initialize Graphics Context");
        return;
    }
    
    this->graphics->listPhysicalDevices();
    logInfo("Created Vulkan Context");
    
    Engine::base = root;
    logInfo("Base Directory: " + Engine::base.string());
}

std::filesystem::path Engine::getAppPath(APP_PATHS appPath) {
    switch(appPath) {
        case SHADERS:
            return Engine::base / "shaders";
        case MODELS:
            return Engine::base / "models";
        case FONTS:
            return Engine::base / "fonts";
        case MAPS:
            return Engine::base / "maps";
        case ROOT:
        default:
            return Engine::base;
    }    
}


bool Engine::isReady() {
    return this->graphics->isGraphicsActive() && this->renderer != nullptr && this->renderer->canRender();
}

void Engine::loop() {
    if (!this->isReady()) return;
    
    logInfo("Starting Render Loop...");

    while(true) {
        this->renderer->drawFrame();
    }
    
    logInfo("Ended Render Loop");
}

void Engine::loadModels() {
    this->models->addModel("TestModel", Engine::getAppPath(MODELS) / "rock.obj");
}

void Engine::init() {
    if(!this->graphics->isGraphicsActive()) return;
    
    this->createRenderer();
    if (this->renderer == nullptr) return;

    renderer->initRenderer();    
    this->createModelPipeline();
}

void Engine::createRenderer() {
    logInfo("Creating Renderer...");

    const std::tuple<VkPhysicalDevice, int> bestRenderDevice = this->graphics->pickBestPhysicalDeviceAndQueueIndex();
    const VkPhysicalDevice defaultPhysicalDevice = std::get<0>(bestRenderDevice);
    if (defaultPhysicalDevice == nullptr) {
        logError("Failed to find suitable physical Device!");
        return;
    }
    
    const int defaultQueueIndex = std::get<1>(bestRenderDevice);
    renderer = new Renderer(this->graphics, defaultPhysicalDevice, defaultQueueIndex);
    
    if (!renderer->isReady()) {
        logError("Failed to initialize Renderer!");
    }
    
    logInfo("Renderer is Ready");
}

void Engine::createModelPipeline() {
    if (this->renderer == nullptr || !renderer->isReady()) return;

    logInfo("Creating Model Pipeline...");

    std::unique_ptr<GraphicsPipeline> pipeline = std::make_unique<GraphicsPipeline>(this->renderer->getLogicalDevice());

    pipeline->addShader(Engine::getAppPath(SHADERS) / "models-vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    pipeline->addShader(Engine::getAppPath(SHADERS) / "models-frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(struct ModelProperties);
    
    if (pipeline->createGraphicsPipeline(
        MAX_FRAMES_IN_FLIGHT, this->renderer->getPhysicalDevice(), this->renderer->getRenderPass(), this->renderer->getCommandPool(), this->renderer->getGraphicsQueue(),
        this->renderer->getSwapChainExtent(), pushConstantRange, this->renderer->doesShowWireFrame())) {
        
        this->renderer->addPipeline(pipeline.release());
    
        logInfo("Added Model Pipeline");
    }
}

Engine::~Engine() {
    if (this->models != nullptr) {
        if (this->renderer != nullptr && this->renderer->getLogicalDevice() != nullptr) {
            this->models->cleanUpTextures(this->renderer->getLogicalDevice());
        }
        delete this->models;
        this->models = nullptr;
    }
    
    if (this->renderer != nullptr) {
        delete this->renderer;
        this->renderer = nullptr;
    }
    
    if (this->graphics != nullptr) {        
        delete this->graphics;
        this->graphics = nullptr;
    }
}
