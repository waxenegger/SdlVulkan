cmake_minimum_required(VERSION 3.20.5)

project(SdlVulkan CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(sdlDir "app/SDL2-2.0.16")
add_subdirectory(${sdlDir} EXCLUDE_FROM_ALL)
set(SDL2MAIN_LIBRARY SDL2main)
set(SDL2_LIBRARY SDL2)

# Sources
set(projectSources
  src/platform.cpp src/desktop.cpp
)

# Include dirs.
set(projectIncludeDirs ${projectIncludeDirs}
    "${sdlDir}/include"
    "glm"
)
include_directories(${projectIncludeDirs})


add_executable(SdlVulkan ${projectSources})

target_link_libraries(${targetName}
  ${SDL2MAIN_LIBRARY}
  ${SDL2_LIBRARY}
)