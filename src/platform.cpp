#include "includes/graphics.h"

constexpr uint32_t VULKAN_VERSION = VK_MAKE_VERSION(1,1,0);

const std::unique_ptr<GraphicsContext> graphics = std::make_unique<GraphicsContext>();
const std::unique_ptr<Models> models = std::make_unique<Models>();
std::unique_ptr<Renderer> renderer;

bool createDefaultRenderer(const VkPhysicalDevice & physicalDevice, const int & queueIndex) {
    renderer = std::make_unique<Renderer>(graphics.get(), physicalDevice, queueIndex);
    if (!renderer->isReady()) {
        logError("Failed to initialize Renderer!");
        return false;
    }
    
    //renderer->addPipeline();
    //pipeline->addShader("/tmp/test.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    
    renderer->initRenderer();
    
    return renderer->canRender();
}

int start(int argc, char* argv []) {
    logInfo("Creating Graphics Context...");
    graphics->initGraphics("Test App", VULKAN_VERSION);

    if(!graphics->isGraphicsActive()) {
        logError("Aborting Program!");

        return -1;
    }
    
    graphics->listPhysicalDevices();
    
    const std::tuple<VkPhysicalDevice, int> bestRenderDevice = graphics->pickBestPhysicalDeviceAndQueueIndex();
    const VkPhysicalDevice defaultPhysicalDevice = std::get<0>(bestRenderDevice);
    if (defaultPhysicalDevice == nullptr) {
        return -1;
    }
    const int defaultQueueIndex = std::get<1>(bestRenderDevice);
    
    if (!createDefaultRenderer(defaultPhysicalDevice, defaultQueueIndex)) {
        logError("Renderer is not ready to render!");
        return -1;
    }
    
    // TODO: read directory maybe or use dynamic process of choosing models
    // when to destroy the texture references held ...
    models->addModel("TestModel", "/opt/projects/SdlVulkan/app/src/main/assets/test.obj");

    return 0;
}

