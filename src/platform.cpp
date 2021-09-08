#include "includes/graphics.h"

constexpr uint32_t VULKAN_VERSION = VK_MAKE_VERSION(1,1,0);

int start(int argc, char* argv []) {
    auto graphics = std::make_unique<GraphicsContext>();
    
    logInfo("Creating Graphics Context...");
    graphics->initGraphics("Test App", VULKAN_VERSION);
    
    if(graphics->isGraphicsActive()) { 
        logInfo("Destroying Graphics Context...");
        graphics->quitGraphics();
    } else {
        logError("Aborting Program!");
        return -1;
    }
    
    return 0;
}
