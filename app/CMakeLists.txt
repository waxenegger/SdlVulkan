cmake_minimum_required(VERSION 3.18.1)

project(SdlVulkanAndroid CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set some helper variables.
string(TOLOWER "${CMAKE_SYSTEM_NAME}" targetSystem)
set(projectDir  "${CMAKE_CURRENT_LIST_DIR}")
set(targetName  "SdlVulkanAndroid")
set(binDir      "${projectDir}/bin")

# Define executable output dir.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${binDir}/${targetSystem}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${binDir}/${targetSystem}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${binDir}/${targetSystem}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${binDir}/${targetSystem}_debug")

# Dependencies:
set(sdlDir "${projectDir}/SDL2-2.0.16")
add_subdirectory(${sdlDir} EXCLUDE_FROM_ALL)
set(SDL2MAIN_LIBRARY SDL2main)
set(SDL2_LIBRARY SDL2)

# Joysticks were causing trouble on android.
add_definitions(-DSDL_JOYSTICK=0)

#find_library( log-lib log )

# Sources (relative to the project root dir).
set(projectSources
  ../src/platform.cpp ../src/android.cpp
)

# Include dirs.
set(projectIncludeDirs ${projectIncludeDirs}
  "${sdlDir}/include" "${projectDir}/../glm"
)

find_library(${SDL2MAIN_LIBRARY}  ${SDL2_LIBRARY})

# Build the binary.
add_library(${targetName} SHARED ${projectSources})

target_link_libraries(${targetName}
  ${SDL2MAIN_LIBRARY}
  ${SDL2_LIBRARY}
  log
)

target_include_directories(${targetName}
  PUBLIC ${projectIncludeDirs}
)


