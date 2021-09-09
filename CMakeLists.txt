cmake_minimum_required(VERSION 3.20.5)

project(SdlVulkan CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDL2_DIR "app/SDL2-2.0.16")
add_subdirectory(${SDL2_DIR} EXCLUDE_FROM_ALL)
find_library(SDL2main SDL2)

#find_package(SDL2 REQUIRED)

set(VulkanDir "VulkanSDK/1.2.189.0/x86_64")
#set(VULKAN_DIR "VulkanSDK/1.2.189.0/win64")
find_package(Vulkan REQUIRED)


# Sources
set(projectSources
  src/platform.cpp 
  src/desktop.cpp
  src/graphics.cpp
  src/pipeline.cpp
)

# Include dirs.
set(projectIncludeDirs ${projectIncludeDirs}
    ${SDL2_DIR}/include
    "glm"
    ${VULKAN_DIR}/include
)
include_directories(${projectIncludeDirs})

add_executable(SdlVulkan ${projectSources})

target_link_libraries(
    ${CMAKE_PROJECT_NAME} 
    SDL2main SDL2 
    Vulkan::Vulkan
)
