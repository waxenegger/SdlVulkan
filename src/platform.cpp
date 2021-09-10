#include "includes/graphics.h"

constexpr uint32_t VULKAN_VERSION = VK_MAKE_VERSION(1,1,0);

const std::unique_ptr<GraphicsContext> graphics = std::make_unique<GraphicsContext>();

int start(int argc, char* argv []) {
    logInfo("Creating Graphics Context...");
    graphics->initGraphics("Test App", VULKAN_VERSION);

    Assimp::Importer importer;

    #ifdef __ANDROID__
        const char * res = SDL_AndroidGetAssetContent("woolly-mammoth-150k.obj");
        const aiScene *scene = importer.ReadFileFromMemory(res, strlen(res),
            aiProcess_Triangulate | aiProcess_GenBoundingBoxes | aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_GenSmoothNormals,
            nullptr);
        free((void *) res);
    #endif

    #ifndef __ANDROID__
        const aiScene *scene = importer.ReadFile("/opt/projects/SdlVulkan/app/src/main/assets/test.obj",    
            aiProcess_Triangulate | aiProcess_GenBoundingBoxes | aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
    #endif

    if (scene == nullptr) {
        logError(importer.GetErrorString() );
    } else {
        logInfo(std::to_string(scene->HasMeshes()));
    }

    if(!graphics->isGraphicsActive()) {
        logError("Aborting Program!");

        return -1;
    }
    
    graphics->listPhysicalDevices();
    
    std::tuple<VkPhysicalDevice, int> defaultRenderDevice = graphics->pickBestPhysicalDeviceAndQueueIndex();


    VkPhysicalDevice defaultPhysicalDevice = std::get<0>(defaultRenderDevice);
    if (defaultPhysicalDevice == nullptr) {
        return -1;
    }

    const std::unique_ptr<GraphicsPipeline> pipeline =
        std::make_unique<GraphicsPipeline>(defaultPhysicalDevice, std::get<1>(defaultRenderDevice));

    pipeline->addShader("/tmp/test.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
        
    return 0;
}
