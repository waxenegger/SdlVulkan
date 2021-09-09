#include "includes/graphics.h"

constexpr uint32_t VULKAN_VERSION = VK_MAKE_VERSION(1,1,0);

const std::unique_ptr<GraphicsContext> graphics = std::make_unique<GraphicsContext>();

int start(int argc, char* argv []) {
    logInfo("Creating Graphics Context...");
    graphics->initGraphics("Test App", VULKAN_VERSION);
    
    if(!graphics->isGraphicsActive()) {
        logError("Aborting Program!");
        return -1;
    }
    
    graphics->listPhysicalDevices();
    
    std::tuple<VkPhysicalDevice, int> defaultRenderDevice = graphics->pickBestPhysicalDeviceAndQueueIndex();


    VkPhysicalDevice defaultPhysicalDevice = std::get<0>(defaultRenderDevice);
    if (defaultPhysicalDevice == nullptr) {
        return -1;
    }

    const std::unique_ptr<GraphicsPipeline> pipeline =
        std::make_unique<GraphicsPipeline>(defaultPhysicalDevice, std::get<1>(defaultRenderDevice));

    //pipeline->addShader("/tmp/test.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    return 0;
}
