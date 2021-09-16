#include "includes/graphics.h"

int start(int argc, char* argv []) {
    const std::unique_ptr<Engine> engine = std::make_unique<Engine>("Test App");

    engine->loadModels();
    engine->createDefaultRenderer();
    
    engine->loop();
    
    return 0;
}

