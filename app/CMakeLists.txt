cmake_minimum_required(VERSION 3.18.1)

project(SdlVulkanAndroid CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_DEBUG_POSTFIX "")

# Set some helper variables.
string(TOLOWER "${CMAKE_SYSTEM_NAME}" targetSystem)
set(projectDir  "${CMAKE_CURRENT_LIST_DIR}")
set(targetName  "SdlVulkanAndroid")

# Dependencies:
set(sdlDir "${projectDir}/SDL2-2.0.16")
add_subdirectory(${sdlDir} EXCLUDE_FROM_ALL)
set(SDL2MAIN_LIBRARY SDL2main)
set(SDL2_LIBRARY SDL2)

set(SDL2_ttf_DIR "${projectDir}/SDL2_ttf-2.0.15")
find_library(SDL_ttf SDL_ttf)

set(SDL2_image_DIR "${projectDir}/SDL2_image-2.0.5")
find_library(SDL_image SDL_image)

set(VulkanDir "VulkanSDK/1.2.189.0/android")

set(AssimpDir "assimp-5.0.1")
add_subdirectory(${AssimpDir} EXCLUDE_FROM_ALL)

# Joysticks were causing trouble on android.
add_definitions(-DSDL_JOYSTICK=0)

#find_library( log-lib log )

# Sources (relative to the project root dir).
set(projectSources
  ../src/android.cpp
  ../src/platform.cpp
  ../src/graphics.cpp
  ../src/pipeline.cpp
  ../src/shader.cpp
  ../src/models.cpp
)

# Include dirs.
set(projectIncludeDirs ${projectIncludeDirs}
  "${sdlDir}/include"
  "${sdlDir}/src/core/android/include"
  ${SDL2_ttf_DIR}/include
  ${SDL2_image_DIR}/include  
  ${VulkanDir}/include
  "${projectDir}/../glm"
  ${ASSIMP_DIR}/include
)

find_library(${SDL2MAIN_LIBRARY}  ${SDL2_LIBRARY})

# Build the binary.
add_library(${targetName} SHARED ${projectSources})

target_link_libraries(${targetName} 
  ${SDL2MAIN_LIBRARY}
  ${SDL2_LIBRARY}
  SDL2_ttf
  SDL2_image
  vulkan
  log
  android
  assimp
)

target_include_directories(${targetName}
  PUBLIC ${projectIncludeDirs}
)


