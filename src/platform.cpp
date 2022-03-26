#include "includes/graphics.h"

std::filesystem::path Engine::base  = "";

int start(int argc, char* argv []) {
    const std::unique_ptr<Engine> engine = std::make_unique<Engine>("Sdl Vulkan App", argc > 1 ? argv[1] : "");

    
    // adding models
    engine->addModel("cyborg", "cyborg.obj");
    engine->addModel("contraption", "contraption.obj");
    engine->addTextModel("text", "FreeMono.ttf", "Hello World", 50);
    engine->addModel("nanosuit", "nanosuit.obj");
    
    
    
    // adding model instances
    Component * rock = Components::INSTANCE()->addComponentFromModel("rock", "rock");
    if (rock != nullptr) {
        rock->setPosition(10.0f,0.0f,10.0f);
        rock->scale(2.0f);
    }

    for (int i=0;i<1;i++) {
        for (int j=0;j<1;j++) {
            Component * nanosuit = Components::INSTANCE()->addComponentFromModel("nanosuit" + std::to_string(i) + "_" + std::to_string(j), "nanosuit");
            if (nanosuit != nullptr) nanosuit->setPosition(0.0f + j*10.0f,10.0f,-11.0f+i* 10.0f);
        }
    }

    Component * text = Components::INSTANCE()->addComponentFromModel("text1", "text");
    if (text != nullptr) {
        text->setPosition(10.0f, 0.0f,20.0f);
        text->rotate(0.0f, 180.0f,0.0f);
    }

    Component * contraption = Components::INSTANCE()->addComponentFromModel("contraption1", "contraption");
    if (contraption != nullptr) {
        contraption->setPosition(10.0f, 30.0f,20.0f);
    }

    Component * cyborg = Components::INSTANCE()->addComponentFromModel("cyborg", "cyborg");
    if (cyborg != nullptr) {
        cyborg->setPosition(0.0f,0.0f,10.0f);
        cyborg->scale(2.0f);
        cyborg->addComponentBehavior(new RandomWalkBehavior(cyborg));
    }
    
    
    // show everything (pretty much)
    engine->setShowSkybox(true);
    engine->setShowScreenMidPoint(true);
    engine->setShowGuiOverlay(true);
    engine->setShowBoundingBoxes(false);

    
    // initialize
    engine->init();

    
    // main loop
    engine->loop();
    
    return 0;
}

