cmake_minimum_required(VERSION 3.20.5)

project(SdlVulkan CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_DEBUG_POSTFIX "")

set(SDL2_DIR "app/SDL2/SDL2-2.0.18")
add_subdirectory(${SDL2_DIR} EXCLUDE_FROM_ALL)
find_library(SDL2main SDL2)

set(SDL_STATIC_ENABLED_BY_DEFAULT, ON)

set(SDL2_ttf_DIR "app/SDL2/SDL2_ttf-2.0.15")
add_subdirectory(${SDL2_ttf_DIR} EXCLUDE_FROM_ALL)
find_library(SDL2_TTF SDL2_ttf)

set(SDL2_image_DIR "app/SDL2/SDL2_image-2.0.5")
add_subdirectory(${SDL2_image_DIR} EXCLUDE_FROM_ALL)
find_library(SDL2_IMAGE SDL2_image)

set(Assimp_DIR "app/assimp-5.0.1")
add_subdirectory(${Assimp_DIR} EXCLUDE_FROM_ALL)
find_library(ASSIMP Assimp)

if (WIN32)
    set(VULKAN_DIR "app/VulkanSDK/1.2.189.0/win64")
    set(VULKAN_LIBRARIES ${PROJECT_SOURCE_DIR}/app/VulkanSDK/1.2.189.0/win64/lib/vulkan-1.lib)
else()
    set(VULKAN_DIR "app/VulkanSDK/1.2.189.0/x86_64")
    set(VULKAN_LIBRARIES vulkan)
endif()

set(IMGUI_DIR "app/imgui-1.86")

# Sources
set(projectSources
    ${IMGUI_DIR}/imgui_impl_sdl.cpp
    ${IMGUI_DIR}/imgui_impl_vulkan.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    src/platform.cpp
    src/desktop.cpp
    src/graphics.cpp
    src/pipeline.cpp
    src/models-pipeline.cpp
    src/skybox-pipeline.cpp
    src/imgui-pipeline.cpp
    src/shader.cpp
    src/models.cpp
    src/renderer.cpp
    src/helper.cpp
    src/engine.cpp
    src/components.cpp
    src/camera.cpp
)

# Include dirs.
set(projectIncludeDirs ${projectIncludeDirs}
    ${SDL2_DIR}/include
    ${SDL2_ttf_DIR}/include
    ${SDL2_image_DIR}/include
    "app/glm"
    ${VULKAN_DIR}/include
    ${ASSIMP_DIR}/include
    ${IMGUI_DIR}
)
include_directories(${projectIncludeDirs})

add_executable(SdlVulkan ${projectSources})

target_link_libraries(
    ${CMAKE_PROJECT_NAME}
    SDL2main
     ${SDL2_LIBRARIES}
    SDL2_ttf
    SDL2_image
    ${VULKAN_LIBRARIES}
    assimp
)
