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
set(VulkanDir "VulkanSDK/1.2.189.0/android")
set(AssimpDir "${projectDir}/assimp-5.0.1")
add_subdirectory(${AssimpDir})

set(sdlDir "${projectDir}/SDL2/SDL2-2.0.16")
add_subdirectory(${sdlDir})
set(SDL2MAIN_LIBRARY SDL2main)
set(SDL2_LIBRARY SDL2)

set(SDL2ttfDir "${projectDir}/SDL2/SDL2_ttf-2.0.15")
add_subdirectory(${SDL2ttfDir})

set(SDL2imageDir "${projectDir}/SDL2/SDL2_image-2.0.5")
add_subdirectory(${SDL2imageDir})


# Joysticks were causing trouble on android.
add_definitions(-DSDL_JOYSTICK=0)

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
  ${SDL2ttfDir}/include
  ${SDL2imageDir}/include
  ${VulkanDir}/include
  "${projectDir}/../glm"
  ${ASSIMP_DIR}/include
)


#find_library( SDL2_TTF SDL2_ttf )
#find_library( SDL2_IMAGE SDL2_image )
#find_library( ${SDL2MAIN_LIBRARY} ${SDL2_LIBRARY} )
#find_library( ASSIMP assimp )

# Build the binary.
add_library(${targetName} SHARED ${projectSources})

target_link_libraries(${targetName}
  vulkan
  log
  android
  assimp
  SDL2_image
  SDL2_ttf
  SDL2::SDL2
)

target_include_directories(${targetName}
  PUBLIC ${projectIncludeDirs}
)



