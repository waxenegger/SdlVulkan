cmake_minimum_required(VERSION 3.20.5)

project(SdlVulkan CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_DEBUG_POSTFIX "")

set(SDL2_DIR "app/SDL2/SDL2-2.0.16")
add_subdirectory(${SDL2_DIR} EXCLUDE_FROM_ALL)
find_library(SDL2main SDL2)


set(SDL2_ttf_DIR "app/SDL2/SDL2_ttf-2.0.15")
add_subdirectory(${SDL2_ttf_DIR} EXCLUDE_FROM_ALL)

set(SDL2_image_DIR "app/SDL2/SDL2_image-2.0.5")
add_subdirectory(${SDL2_image_DIR} EXCLUDE_FROM_ALL)

set(Assimp_DIR "app/assimp-5.0.1")
add_subdirectory(${Assimp_DIR} EXCLUDE_FROM_ALL)
find_package(Assimp REQUIRED)


set(VulkanDir "VulkanSDK/1.2.189.0/x86_64")
#set(VULKAN_DIR "VulkanSDK/1.2.189.0/win64")
find_package(Vulkan REQUIRED)


# Sources
set(projectSources
    src/platform.cpp
    src/desktop.cpp
    src/graphics.cpp
    src/pipeline.cpp
    src/shader.cpp
    src/models.cpp
)

# Include dirs.
set(projectIncludeDirs ${projectIncludeDirs}
    ${SDL2_DIR}/include
    ${SDL2_ttf_DIR}/include
    ${SDL2_image_DIR}/include
    "glm"
    ${VULKAN_DIR}/include
    ${ASSIMP_DIR}/include
)
include_directories(${projectIncludeDirs})

add_executable(SdlVulkan ${projectSources})

target_link_libraries(
    ${CMAKE_PROJECT_NAME} 
    SDL2main SDL2 
    SDL2_ttf
    SDL2_image
    Vulkan::Vulkan
    assimp
)
