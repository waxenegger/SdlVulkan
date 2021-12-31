#include "includes/graphics.h"

Engine::Engine(const std::string & appName, const std::string root, const uint32_t version) {
    logInfo("Creating Graphics Context...");
    this->graphics->initGraphics("Test App", VULKAN_VERSION);

    if(!this->graphics->isGraphicsActive()) {
        logError("Could not initialize Graphics Context");

        #ifdef __ANDROID__
                SDL_AndroidShowToast("Vulkan Not Supported", 1, 0, 0, 0);
                std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        #endif

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
        case SKYBOX:
            return Engine::base / "skybox";
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
    
    SDL_StartTextInput();

    this->startInputCapture();
    
    logInfo("Starting Render Loop...");

    while(!this->quit) {
        this->renderer->drawFrame();
    }
    
    SDL_StopTextInput();
    
    logInfo("Ended Render Loop");
}

void Engine::preloadModels() {
    // TODO: read from location
    this->models->addModel("cyborg", Engine::getAppPath(MODELS) / "cyborg.obj");
    this->models->addModel("rock", Engine::getAppPath(MODELS) / "rock.obj");
    this->models->addModel("nanosuit", Engine::getAppPath(MODELS) / "nanosuit.obj");
    
    this->components->initWithModelIds(this->models->getModelIds());
}

void Engine::updateModels(const std::string id, const std::filesystem::path file) {
    if (this->renderer == nullptr || !renderer->isReady()) return;

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    
    logInfo("Adding Model...");
    
    Models::INSTANCE()->removeDummyTexture(this->renderer->getLogicalDevice());
    if (!this->models->addModel(id, file)) {
        Models::INSTANCE()->addDummyTexture(this->renderer->getSwapChainExtent());
        return;
    }
    
    this->updateModelPipeline();
    
    this->renderer->updateRenderer();
    
    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> time_span = now - start;
    logInfo("Duration Update Models: " + std::to_string(time_span.count()));
}

void Engine::init() {
    if(!this->graphics->isGraphicsActive()) return;
    
    this->createRenderer();
    if (this->renderer == nullptr) return;

    renderer->initRenderer();    
    
    this->createSkyboxPipeline();
    this->createModelPipeline();
    
    VkExtent2D windowSize = this->renderer->getSwapChainExtent();
    this->camera->setAspectRatio(static_cast<float>(windowSize.width) / windowSize.height);
    
    this->renderer->updateRenderer();
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

    std::unique_ptr<GraphicsPipeline> pipeline = std::make_unique<ModelsPipeline>(this->renderer);

    pipeline->addShader((Engine::getAppPath(SHADERS) / "models-vert.spv").string(), VK_SHADER_STAGE_VERTEX_BIT);
    pipeline->addShader((Engine::getAppPath(SHADERS) / "models-frag.spv").string(), VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(struct ModelProperties);
    
    if (pipeline->createGraphicsPipeline(pushConstantRange)) {
        
        this->modelPipelineIndex = this->renderer->addPipeline(pipeline.release());
    
        logInfo("Added Model Pipeline");
    }
}

void Engine::createSkyboxPipeline() {
    if (this->renderer == nullptr || !renderer->isReady()) return;    

    logInfo("Creating Skybox Pipeline...");

    std::unique_ptr<GraphicsPipeline> pipeline = std::make_unique<SkyboxPipeline>(this->renderer);

    pipeline->addShader((Engine::getAppPath(SHADERS) / "skybox-vert.spv").string(), VK_SHADER_STAGE_VERTEX_BIT);
    pipeline->addShader((Engine::getAppPath(SHADERS) / "skybox-frag.spv").string(), VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPushConstantRange pushConstantRange{};
    
    if (pipeline->createGraphicsPipeline(pushConstantRange)) {
        
        this->renderer->addPipeline(pipeline.release());
    
        logInfo("Added Skybox Pipeline");
    }
}

void Engine::updateModelPipeline() {
    if (this->renderer == nullptr || !renderer->isReady() || !renderer->canRender()) return;    

    logInfo("Updating Model Pipeline...");

    GraphicsPipeline * pipeline = this->renderer->getPipeline(this->modelPipelineIndex);
    pipeline->destroyPipelineObjects();

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(struct ModelProperties);
    
    if (pipeline->createGraphicsPipeline(pushConstantRange)) {
    
        logInfo("Updated Model Pipeline");
    }
}

void Engine::startInputCapture() {
    
    std::thread inputThread([this]() {
        SDL_Event e;
        bool isFullScreen = false;
        bool needsRestoreAfterFullScreen = false;

        float walkingSpeed = 0.5;
        
        while(!this->quit) {
            while (SDL_PollEvent(&e) != 0) {
                switch(e.type) {
                    case SDL_WINDOWEVENT:
                        if (e.window.event == SDL_WINDOWEVENT_RESIZED ||
                            e.window.event == SDL_WINDOWEVENT_MAXIMIZED ||
                            e.window.event == SDL_WINDOWEVENT_MINIMIZED ||
                            e.window.event == SDL_WINDOWEVENT_RESTORED) {
                                if (isFullScreen) SDL_SetWindowFullscreen(this->graphics->getSdlWindow(), SDL_TRUE);
                        }
                        break;
                    case SDL_KEYDOWN:
                        switch (e.key.keysym.scancode) {
                            case SDL_SCANCODE_W:
                                this->camera->move(Camera::KeyPress::UP, true, walkingSpeed);
                                break;
                            case SDL_SCANCODE_S:
                                this->camera->move(Camera::KeyPress::DOWN, true, walkingSpeed);
                                break;
                            case SDL_SCANCODE_A:
                                this->camera->move(Camera::KeyPress::LEFT, true, walkingSpeed);
                                break;
                            case SDL_SCANCODE_D:
                                this->camera->move(Camera::KeyPress::RIGHT, true, walkingSpeed);
                                break;
                            case SDL_SCANCODE_F:{
                                this->renderer->setShowWireFrame(!this->renderer->doesShowWireFrame());
                                break;           
                            }
                            case SDL_SCANCODE_F12:
                                isFullScreen = !isFullScreen;
                                if (isFullScreen) {
                                    if (SDL_GetWindowFlags(this->graphics->getSdlWindow()) & SDL_WINDOW_MAXIMIZED) {
                                        SDL_SetWindowFullscreen(this->graphics->getSdlWindow(), SDL_TRUE);
                                    } else {
                                        needsRestoreAfterFullScreen = true;
                                        SDL_MaximizeWindow(this->graphics->getSdlWindow());
                                    }
                                } else {
                                    SDL_SetWindowFullscreen(this->graphics->getSdlWindow(), SDL_FALSE);
                                    if (needsRestoreAfterFullScreen) {
                                        SDL_RestoreWindow(this->graphics->getSdlWindow());
                                        needsRestoreAfterFullScreen = false;
                                    }
                                }
                                break;
                            case SDL_SCANCODE_Q:
                                quit = true;
                                break;
                            default:
                                break;
                        };
                        break;
                    case SDL_KEYUP:
                        switch (e.key.keysym.scancode) {
                            case SDL_SCANCODE_W:
                                this->camera->move(Camera::KeyPress::UP);
                                break;
                            case SDL_SCANCODE_S:
                                this->camera->move(Camera::KeyPress::DOWN);
                                break;
                            case SDL_SCANCODE_A:
                                this->camera->move(Camera::KeyPress::LEFT);
                                break;
                            case SDL_SCANCODE_D:
                                this->camera->move(Camera::KeyPress::RIGHT);
                                break;
                            default:
                                break;
                        };
                        break;
                    case SDL_MOUSEMOTION:
                        if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
                            this->camera->updateDirection(
                                static_cast<float>(e.motion.xrel),
                                static_cast<float>(e.motion.yrel), 0.005f);
                        }
                        break;
                    case SDL_MOUSEWHEEL:
                    {
                        const Sint32 delta = e.wheel.y * (e.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? 1 : -1);
                        float newFovy = this->camera->getFovY() - delta * 2;
                        if (newFovy < 1) newFovy = 1;
                        else if (newFovy > 45) newFovy = 45;
                        this->camera->setFovY(newFovy);
                        break;
                    }                            
                    case SDL_MOUSEBUTTONUP:
                        SDL_SetRelativeMouseMode(SDL_GetRelativeMouseMode() == SDL_TRUE ? SDL_FALSE : SDL_TRUE);
                        break;
                    case SDL_QUIT:
                        quit = true;
                        break;
                }
            }
        }
    });
    inputThread.detach();
}



Engine::~Engine() {    
    if (this->renderer != nullptr) {
        delete this->renderer;
        this->renderer = nullptr;
    }
    
    if (this->models != nullptr) {
        delete this->models;
        this->models = nullptr;
    }

    if (this->graphics != nullptr) {        
        delete this->graphics;
        this->graphics = nullptr;
    }
    
    Camera::INSTANCE()->destroy();
    
    if (this->components != nullptr) {
        delete this->components;
        this->components = nullptr;
    }
}
