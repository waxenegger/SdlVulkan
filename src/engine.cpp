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
    this->renderer->resume();
    
    SDL_StartTextInput();

    if (USE_THREADED_INPUT) {
        this->startInputCapture();
    }
    
    logInfo("Starting Render Loop...");

    if (USE_THREADED_INPUT) {
        this->startInputCapture();
        
        while(!this->quit) {
            this->renderer->drawFrame();
        }
    } else {
        this->inputLoop();
    }

    
    SDL_StopTextInput();
    
    logInfo("Ended Render Loop");
}

bool Engine::beforeModelAddition() {
    logInfo("Adding Model...");

    bool hasBeenPaused = true;
    
    if (this->renderer != nullptr && renderer->isReady() && !renderer->isPaused()) {
        this->renderer->pause();
    } else hasBeenPaused = false;

    return hasBeenPaused;
}


void Engine::afterModelAddition(const bool resume, const std::string id) {
    logInfo("Added Model " + id);

    if (this->renderer == nullptr || !renderer->isReady()) return;
                
    GraphicsPipeline * pipeline = this->renderer->getPipeline(this->modelPipelineIndex);
    if (pipeline != nullptr) {
        ((ModelsPipeline *) pipeline)->updateModels();
    }
    
    if (resume) this->renderer->resume();
}

void Engine::addModel(Model * model) {
    if (model == nullptr) return;
        
    bool hasBeenPaused = this->beforeModelAddition();
    
    if (!this->models->addModel(model)) {
        this->renderer->resume();
        return;
    }
    
    this->afterModelAddition(hasBeenPaused, model->getId());
}

void Engine::addModel(const std::string id, const std::filesystem::path file) {
    bool hasBeenPaused = this->beforeModelAddition();
    
    if (!this->models->addModel(id, Engine::getAppPath(MODELS) / file)) {
        this->renderer->resume();
        return;
    }
    
    this->afterModelAddition(hasBeenPaused, id);
}

void Engine::addTextModel(std::string id, const std::filesystem::path font, std::string text, uint16_t size) {
    bool hasBeenPaused = this->beforeModelAddition();
    
    if (!this->models->addTextModel(id, Engine::getAppPath(FONTS) / font, text, size)) {
        this->renderer->resume();
        return;
    }
    
    this->afterModelAddition(hasBeenPaused, id);
}

void Engine::init() {
    if(!this->graphics->isGraphicsActive()) return;
    
    this->createRenderer();
    if (this->renderer == nullptr) return;

    renderer->initRenderer();
    
    if (this->showSkybox) {
        this->createSkyboxPipeline();
    }

    if (this->showScreenMidPoint) {
        this->createScreenMidPointPipeline();
    }

    if (this->showComponents) {
        this->createModelPipeline();
    }

    if (this->showGuiOverlay) {
        this->createImGuiPipeline();
    }

    this->setShowBoundingBoxes(this->showBoundingBoxes);
    
    VkExtent2D windowSize = this->renderer->getSwapChainExtent();
    this->camera->setAspectRatio(static_cast<float>(windowSize.width) / windowSize.height);
    
    this->renderer->updateRenderer();
}

float Engine::getDeltaFactor() {
    if (this->renderer == nullptr) return DELTA_TIME_60FPS;
        
    return this->renderer->getDeltaFactor();
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

    pipeline->addShader((Engine::getAppPath(SHADERS) / "models-memory-vert.spv").string(), VK_SHADER_STAGE_VERTEX_BIT);
    pipeline->addShader((Engine::getAppPath(SHADERS) / "models-memory-frag.spv").string(), VK_SHADER_STAGE_FRAGMENT_BIT);

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
        
        this->skyboxPipelineIndex = this->renderer->addPipeline(pipeline.release());
    
        logInfo("Added Skybox Pipeline");
    }
}

void Engine::createScreenMidPointPipeline() {
    if (this->renderer == nullptr || !renderer->isReady()) return;    

    logInfo("Creating Screen Midpoint Pipeline...");

    std::unique_ptr<GraphicsPipeline> pipeline = std::make_unique<ScreenMidPointPipeline>(this->renderer);

    pipeline->addShader((Engine::getAppPath(SHADERS) / "midpoint-vert.spv").string(), VK_SHADER_STAGE_VERTEX_BIT);
    pipeline->addShader((Engine::getAppPath(SHADERS) / "midpoint-frag.spv").string(), VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPushConstantRange pushConstantRange{};
    
    if (pipeline->createGraphicsPipeline(pushConstantRange)) {
        
        this->screenMidPointPipelineIndex = this->renderer->addPipeline(pipeline.release());
    
        logInfo("Added Screen Midpoint Pipeline");
    }
}

void Engine::createImGuiPipeline() {
    if (this->renderer == nullptr || !renderer->isReady()) return;    

    logInfo("Creating ImGui Pipeline...");

    std::unique_ptr<ImGuiPipeline> pipeline = std::make_unique<ImGuiPipeline>(this->renderer);

    VkPushConstantRange pushConstantRange{};
    
    if (pipeline->createGraphicsPipeline(pushConstantRange)) {
        
        this->guiPipelineIndex = this->renderer->addPipeline(pipeline.release());
    
        logInfo("Added ImGui Pipeline");
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

void Engine::inputLoop() {
    SDL_Event e;
    bool isFullScreen = false;
    bool needsRestoreAfterFullScreen = false;

    float walkingSpeed = 0.5f;
    float cameraSpeed = 0.0015f;
    
    std::function<bool(BoundingBox)> collisionFunc = std::bind(&Components::checkCollision, Components::INSTANCE(), std::placeholders::_1);
    
    while(!this->quit) {
        while (SDL_PollEvent(&e) != 0) {
            switch(e.type) {
                case SDL_WINDOWEVENT:
                    if (e.window.event == SDL_WINDOWEVENT_RESIZED ||
                        e.window.event == SDL_WINDOWEVENT_MAXIMIZED ||
                        e.window.event == SDL_WINDOWEVENT_MINIMIZED ||
                        e.window.event == SDL_WINDOWEVENT_RESTORED) {
                            if (this->renderer != nullptr && !this->renderer->isPaused()) {
                                if (isFullScreen && this->renderer->isMaximized()) {
                                    SDL_SetWindowFullscreen(this->graphics->getSdlWindow(), SDL_WINDOW_FULLSCREEN);
                                }
                            }
                    }
                    break;
                case SDL_KEYDOWN:
                    switch (e.key.keysym.scancode) {
                        case SDL_SCANCODE_W:
                            this->camera->move(Camera::KeyPress::UP, true, walkingSpeed, collisionFunc);
                            break;
                        case SDL_SCANCODE_S:
                            this->camera->move(Camera::KeyPress::DOWN, true, walkingSpeed, collisionFunc);
                            break;
                        case SDL_SCANCODE_A:
                            this->camera->move(Camera::KeyPress::LEFT, true, walkingSpeed, collisionFunc);
                            break;
                        case SDL_SCANCODE_D:
                            this->camera->move(Camera::KeyPress::RIGHT, true, walkingSpeed, collisionFunc);
                            break;
                        case SDL_SCANCODE_F:
                            this->renderer->setShowWireFrame(!this->renderer->doesShowWireFrame());
                            break;           
                        case SDL_SCANCODE_M:
                            this->showComponents = !this->showComponents;
                            this->renderer->enablePipeline(this->modelPipelineIndex, this->showComponents);
                            break;           
                        case SDL_SCANCODE_G:
                            this->showGuiOverlay = !this->showGuiOverlay;
                            this->renderer->enablePipeline(this->guiPipelineIndex, this->showGuiOverlay);
                            break;           
                        case SDL_SCANCODE_B:
                            this->setShowBoundingBoxes(!this->showBoundingBoxes);
                            break;
                        case SDL_SCANCODE_F12:
                            if (this->renderer != nullptr && !this->renderer->isPaused()) {
                                isFullScreen = !this->renderer->isFullScreen();
                                if (isFullScreen) {
                                    if (this->renderer->isMaximized()) {
                                        SDL_SetWindowFullscreen(this->graphics->getSdlWindow(), SDL_WINDOW_FULLSCREEN);
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
                            static_cast<float>(e.motion.yrel), cameraSpeed);
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

        if (!USE_THREADED_INPUT) {
            this->renderer->drawFrame();
        }
    }
}

void Engine::startInputCapture() {
    std::thread inputThread(&Engine::inputLoop, this);
    inputThread.detach();
}

void Engine::setShowSkybox(const bool flag) {
    this->showSkybox = flag;
}

void Engine::setShowScreenMidPoint(const bool flag) {
    this->showScreenMidPoint = flag;
}

void Engine::setShowComponents(const bool flag) {
    this->showComponents = flag;
}

void Engine::setShowGuiOverlay(const bool flag) {
    this->showGuiOverlay = flag;
}

void Engine::setShowBoundingBoxes(const bool flag) {
    this->showBoundingBoxes = flag;
    
    if (this->renderer == nullptr) return;
    
    GraphicsPipeline * modelPipeline = renderer->getPipeline(this->modelPipelineIndex);
    if (modelPipeline != nullptr) {
        modelPipeline->setShowBoundingBoxes(this->showBoundingBoxes);
    }
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
}
