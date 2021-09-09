cmake_minimum_required(VERSION 3.20.5)

project(SdlVulkan CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

set(VulkanDir "VulkanSDK/1.2.189.0/x86_64")

# Sources
set(projectSources
  src/platform.cpp 
  src/desktop.cpp
  src/graphics.cpp
  src/pipeline.cpp
)

# Include dirs.
set(projectIncludeDirs ${projectIncludeDirs}
    ${SDL2_INCLUDE_DIRS}
    "glm"
    ${VulkanDir}/include
)
include_directories(${projectIncludeDirs})

add_executable(SdlVulkan ${projectSources})

target_link_libraries(
    ${CMAKE_PROJECT_NAME} 
    ${SDL2_LIBRARIES} 
    vulkan
)
