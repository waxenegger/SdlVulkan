#ifndef SRC_INCLUDES_SHARED_INCL_H_
#define SRC_INCLUDES_SHARED_INCL_H_

#include <SDL.h>

#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <iostream>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/string_cast.hpp>

#define ASSERT_VULKAN(val) \
    if (val != VK_SUCCESS) { \
        std::cerr << "Error: " << val << std::endl; \
    };

int start(int argc, char* argv []);

#endif
