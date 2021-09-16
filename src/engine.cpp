#include "includes/graphics.h"

Engine::Engine(const std::string & appName, const uint32_t version) {
    logInfo("Creating Graphics Context...");
    this->graphics->initGraphics("Test App", VULKAN_VERSION);

    if(!this->graphics->isGraphicsActive()) {
        logError("Could not initialize Graphics Context");
        return;
    }
    
    this->graphics->listPhysicalDevices();
    logInfo("Created Vulkan Context");
}


bool Engine::isReady() {
    return this->graphics->isGraphicsActive() && this->renderer != nullptr && this->renderer->canRender();
}

void Engine::loop() {
    if (!this->isReady()) return;
    
    while(true) {
        this->renderer->drawFrame();
    }
}

void Engine::loadModels() {
    this->models->addModel("TestModel", "/opt/projects/SdlVulkan/app/src/main/assets/test.obj");
}

void Engine::createDefaultRenderer() {
    if(!this->graphics->isGraphicsActive()) return;
    
    const std::tuple<VkPhysicalDevice, int> bestRenderDevice = this->graphics->pickBestPhysicalDeviceAndQueueIndex();
    const VkPhysicalDevice defaultPhysicalDevice = std::get<0>(bestRenderDevice);
    if (defaultPhysicalDevice == nullptr) return;
    
    const int defaultQueueIndex = std::get<1>(bestRenderDevice);
    renderer = new Renderer(this->graphics, defaultPhysicalDevice, defaultQueueIndex);
    if (!renderer->isReady()) {
        logError("Failed to initialize Renderer!");
        return;
    }
    
    //renderer->addPipeline();
    //pipeline->addShader("/tmp/test.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    
    renderer->initRenderer();
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
