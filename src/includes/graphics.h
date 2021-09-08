#ifndef SRC_INCLUDES_GRAPHICS_INCL_H_
#define SRC_INCLUDES_GRAPHICS_INCL_H_

#include "shared.h"

class GraphicsContext final {
    private:
        SDL_Window * sdlWindow = nullptr;
        VkInstance vulkanInstance = nullptr;
        VkSurfaceKHR vulkanSurface = nullptr;
        
        std::vector<const char *> vkExtensionNames;
        std::vector<const char *> vkLayerNames = {
           //"VK_LAYER_KHRONOS_validation"
        };

        bool queryVulkanInstanceExtensions();
        void createVulkanInstance(const std::string & appName, const uint32_t version);

    public:
        bool isSdlActive() const;
        bool isVulkanActive() const;
        bool isGraphicsActive() const;
        
        void initSdl(const std::string & appName);
        void initVulkan(const std::string & appName, const uint32_t version);
        void initGraphics(const std::string & appName, const uint32_t version);
        
        void quitSdl();
        void quitVulkan();
        void quitGraphics();
};

#endif


