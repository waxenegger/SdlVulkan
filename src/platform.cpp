#include "includes/graphics.h"

std::filesystem::path Engine::base  = "";

int start(int argc, char* argv []) {
    const std::unique_ptr<Engine> engine = std::make_unique<Engine>("Test App", argc > 1 ? argv[1] : "");

    engine->loadModels();
    engine->init();
    
    engine->loop();
    
    return 0;
}

