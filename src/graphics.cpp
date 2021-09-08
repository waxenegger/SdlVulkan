#include "includes/graphics.h"


void GraphicsContext::initSdl(const std::string & appName) {
    if (this->isSdlActive()) return;
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        logError("Could not initialize SDL! Error: " + std::string(SDL_GetError()));
    }
    this->sdlWindow =
            SDL_CreateWindow(appName.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (this->sdlWindow == nullptr) {
        logError("SDL Window could not be created! Error: " + std::string(SDL_GetError()));
    }
}

void GraphicsContext::createVulkanInstance(const std::string & appName, const uint32_t version) {
    if (this->isVulkanActive()) return;
    
    VkApplicationInfo app;
    app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.pNext = nullptr,
    app.pApplicationName = appName.c_str();
    app.applicationVersion = version;
    app.pEngineName = appName.c_str();
    app.engineVersion = version;
    app.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo inst_info;
    inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_info.pNext = nullptr;
    inst_info.flags = 0;
    inst_info.pApplicationInfo = &app;
    inst_info.enabledLayerCount = this->vkLayerNames.size();
    inst_info.ppEnabledLayerNames = !this->vkLayerNames.empty() ? this->vkLayerNames.data() : nullptr;
    inst_info.enabledExtensionCount = this->vkExtensionNames.size();
    inst_info.ppEnabledExtensionNames = !this->vkExtensionNames.empty() ? this->vkExtensionNames.data() : nullptr;

    const VkResult ret = vkCreateInstance(&inst_info, nullptr, &this->vulkanInstance);
    if (ret != VK_SUCCESS) {
        logError("Failed to create Vulkan Instance");
        return;
    }
    
    this->queryVulkanInstanceExtensions();
}

bool GraphicsContext::queryVulkanInstanceExtensions() {
    if (!this->isSdlActive()) return false;
    
    uint32_t extensionCount = 0;
    if (SDL_Vulkan_GetInstanceExtensions(this->sdlWindow, &extensionCount, nullptr) == SDL_FALSE) {
        logError("Could not get SDL Vulkan Extensions: " + std::string(SDL_GetError()));
        return false;
    } else {
        this->vkExtensionNames.resize(extensionCount);
        return SDL_Vulkan_GetInstanceExtensions(this->sdlWindow, &extensionCount, this->vkExtensionNames.data());
    }
}

void GraphicsContext::initVulkan(const std::string & appName, const uint32_t version) {
    if (this->isVulkanActive()) return;
    
    if (!this->isSdlActive()) {
        this->initSdl(appName);
        if (!this->isSdlActive()) return;
    }
    
    if (!this->queryVulkanInstanceExtensions()) return;
    
    this->createVulkanInstance(appName, version);
    if (this->vulkanInstance == nullptr) return;
    
    if (SDL_Vulkan_CreateSurface(this->sdlWindow, this->vulkanInstance, &this->vulkanSurface) == SDL_FALSE) {
        logError("Failed to Create Vulkan Surface: " + std::string(SDL_GetError()));
        this->quitVulkan();
    }
}

void GraphicsContext::initGraphics(const std::string & appName, const uint32_t version) {
    if (this->isGraphicsActive()) return;
    
    this->initSdl(appName);
    this->initVulkan(appName, version);
}

bool GraphicsContext::isSdlActive() const {
    return this->sdlWindow != nullptr;
}

bool GraphicsContext::isVulkanActive() const {
    return this->vulkanInstance != nullptr && this->vulkanSurface != nullptr;
}

bool GraphicsContext::isGraphicsActive() const {
    return this->isSdlActive() && this->isVulkanActive();
}

void GraphicsContext::quitSdl() {
    if (this->sdlWindow != nullptr) {
        SDL_DestroyWindow(this->sdlWindow);
        this->sdlWindow = nullptr;
    }
    
    SDL_Quit();
}

void GraphicsContext::quitVulkan() {
    if (this->vulkanSurface != nullptr) {
        vkDestroySurfaceKHR(this->vulkanInstance, this->vulkanSurface, nullptr);
        this->vulkanSurface = nullptr;
    }

    if (this->vulkanInstance != nullptr) {
        vkDestroyInstance(this->vulkanInstance, nullptr);
        this->vulkanInstance = nullptr;
    }
}

void GraphicsContext::quitGraphics() {
    this->quitVulkan();
    this->quitSdl();
}
