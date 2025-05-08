cmake_minimum_required(VERSION 3.16)
project(Nebulite)

# Echo the start of the configuration
message(STATUS "Starting configuration for project Nebulite")

# Specify where to store the compiled binaries for Linux
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Application/bin)
message(STATUS "Binary output directory set to: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

# Set paths to local SDL2 and RapidJSON libraries
set(SDL2_PATH       "${CMAKE_SOURCE_DIR}/external/SDL2_build")
set(RAPIDJSON_PATH  "${CMAKE_SOURCE_DIR}/external/rapidjson")
set(EXPRTK_PATH     "${CMAKE_SOURCE_DIR}/external/exprtk")
set(TINYEXPR_PATH   "${CMAKE_SOURCE_DIR}/external/tinyexpr")

# Define SDL_DISABLE_IMPLEMENTS to use old names (for compatibility)
add_definitions(-DSDL_DISABLE_IMPLEMENTS)

# Set C++ standard for Linux
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message(STATUS "C++20 standard set")

add_compile_options(-g -w)
message(STATUS "Added compiler options: -g -w")

#####################
# [INCLUDES]

# Include directories common for both platforms
message(STATUS "Setting Include directories")
include_directories(
    ${CMAKE_SOURCE_DIR}/src/Engine
    ${CMAKE_SOURCE_DIR}/src/Engine/Rendering
    ${CMAKE_SOURCE_DIR}/src/Engine/Helper
    ${CMAKE_SOURCE_DIR}/src/Platform
    ${CMAKE_SOURCE_DIR}/src/Editor
    ${CMAKE_SOURCE_DIR}/src/Tests
    ${SDL2_PATH}/include/SDL2/
    ${RAPIDJSON_PATH}/include/rapidjson
    ${EXPRTK_PATH}/
    ${TINYEXPR_PATH}/
)

#####################
# [SOURCES]

# Common source files
message(STATUS "Setting source files in project")
set(COMMON_SOURCES
    ${TINYEXPR_PATH}/tinyexpr.c
    ${CMAKE_SOURCE_DIR}/src/Platform/Platform.cpp
    #${CMAKE_SOURCE_DIR}/src/Editor/Editor.cpp
    #${CMAKE_SOURCE_DIR}/src/Editor/LevelEditor.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Helper/FileManagement.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Helper/FuncTree.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Helper/Invoke.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Helper/JSONHandler.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Helper/StringHandler.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Helper/Time.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Rendering/Environment.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Rendering/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Rendering/RenderObject.cpp
    ${CMAKE_SOURCE_DIR}/src/Tests/DsaDebug.cpp
    ${CMAKE_SOURCE_DIR}/src/Tests/TestEnv.cpp
)

#####################
# [OUTPUT]

message(STATUS "Setting targets...")

# Engine target
add_executable(Nebulite
    ${CMAKE_SOURCE_DIR}/src/Engine/main.cpp
    ${COMMON_SOURCES}
)
target_link_libraries(Nebulite PRIVATE 
    ${SDL2_PATH}/lib/libSDL2.a 
    ${SDL2_PATH}/lib/libSDL2_ttf.a
    ${SDL2_PATH}/lib/libSDL2_image.a
)
