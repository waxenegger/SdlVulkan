#include "includes/graphics.h"

std::filesystem::path Engine::base  = "";

int start(int argc, char* argv []) {
    const std::unique_ptr<Engine> engine = std::make_unique<Engine>("Sdl Vulkan App", argc > 1 ? argv[1] : "");

    engine->loadModels();
    
    
    // place a test object
    Component * rock = Components::INSTANCE()->addComponentFromModel("rock instance", "rock");
    rock->setPosition(0.0f,0.0f,-10.0f);
    
    
    engine->init();
    
    engine->loop();
    
    return 0;
}

