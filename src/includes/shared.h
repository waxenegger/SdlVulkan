#ifndef SRC_INCLUDES_SHARED_INCL_H_
#define SRC_INCLUDES_SHARED_INCL_H_

#include "logging.h"

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>
#include <array>
#include <memory>
#include <map>
#include <fstream>
#include <filesystem>
#include <string>

#include <thread>
#include <mutex>
#include <queue>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/string_cast.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

int start(int argc, char* argv []);

static constexpr uint32_t MEGA_BYTE = 1000 * 1000;


enum APP_PATHS {
    ROOT, SHADERS, MODELS, FONTS, MAPS
};

#endif
