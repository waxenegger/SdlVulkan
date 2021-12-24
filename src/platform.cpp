#include "includes/graphics.h"

std::filesystem::path Engine::base  = "";

int start(int argc, char* argv []) {
    const std::unique_ptr<Engine> engine = std::make_unique<Engine>("Sdl Vulkan App", argc > 1 ? argv[1] : "");

   std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    engine->preloadModels();

    // place a test object
    for (int i=0;i<1000;i++) {
        for (int j=0;j<10;j++) {
            Component * cyborg = Components::INSTANCE()->addComponentFromModel("cyborg instance" + std::to_string(i) + "_" + std::to_string(j),     "nanosuit");
            if (cyborg != nullptr) cyborg->setPosition(0.0f + j*10.0f,10.0f,-11.0f+i* 10.0f);
        }
    }
        
    engine->init();
    
    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> time_span = now - start;
    std::cout << "Duration Init: " << time_span.count() << std::endl;

    engine->loop();
    
    return 0;
}

