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
#include <functional>
#include <condition_variable>
#include <atomic>
#include <random>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/string_cast.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_vulkan.h"

int start(int argc, char* argv []);

static constexpr bool USE_THREADS = true;
static constexpr bool USE_THREADED_INPUT = true;
static constexpr bool USE_SSBO_MEMORY = false;

static constexpr uint32_t MAX_BUFFERING = 3;
static constexpr uint32_t MIPMAP_LEVELS = 8;
static constexpr uint64_t IMAGE_ACQUIRE_TIMEOUT = 5 * 1000;

static constexpr int FRAME_RATE_60 = 60;
static constexpr double DELTA_TIME_60FPS = 1000.0f / FRAME_RATE_60;

static constexpr uint64_t KILO_BYTE = 1000;
static constexpr uint64_t MEGA_BYTE = KILO_BYTE * 1000;
static constexpr uint64_t GIGA_BYTE = MEGA_BYTE * 1000;

static constexpr float INF = std::numeric_limits<float>::infinity();
static constexpr float NEG_INF = -1 * std::numeric_limits<float>::infinity();

enum APP_PATHS {
    ROOT, SHADERS, MODELS, SKYBOX, FONTS, MAPS
};

typedef unsigned int uint;

#endif
