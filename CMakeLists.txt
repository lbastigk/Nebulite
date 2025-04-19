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
    ${CMAKE_SOURCE_DIR}/Engine
    ${CMAKE_SOURCE_DIR}/Engine/Rendering
    ${CMAKE_SOURCE_DIR}/Engine/GameLoop
    ${CMAKE_SOURCE_DIR}/Engine/Helper
    ${CMAKE_SOURCE_DIR}/Engine/GameVars
    ${CMAKE_SOURCE_DIR}/Platform
    ${CMAKE_SOURCE_DIR}/Editor
    ${CMAKE_SOURCE_DIR}/Tests
    ${SDL2_PATH}/include/SDL2/
    ${RAPIDJSON_PATH}/include/rapidjson
    ${EXPRTK_PATH}/
)

#####################
# [SOURCES]

# Common source files
message(STATUS "Setting source files in project")
set(COMMON_SOURCES
    ${CMAKE_SOURCE_DIR}/Platform/Platform.cpp
    ${CMAKE_SOURCE_DIR}/Editor/Editor.cpp
    ${CMAKE_SOURCE_DIR}/Editor/LevelEditor.cpp
    ${CMAKE_SOURCE_DIR}/Engine/GameVars/Basiswerte.cpp
    ${CMAKE_SOURCE_DIR}/Engine/GameVars/Eigenschaften.cpp
    ${CMAKE_SOURCE_DIR}/Engine/GameVars/InteractionRequirement.cpp
    ${CMAKE_SOURCE_DIR}/Engine/GameVars/Inventar.cpp
    ${CMAKE_SOURCE_DIR}/Engine/GameVars/Kreatur.cpp
    ${CMAKE_SOURCE_DIR}/Engine/GameVars/Quest.cpp
    ${CMAKE_SOURCE_DIR}/Engine/GameVars/Talente.cpp
    ${CMAKE_SOURCE_DIR}/Engine/GameVars/Zauber.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Helper/Calculator.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Helper/Console.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Helper/FileManagement.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Helper/FuncTree.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Helper/Invoke.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Helper/JSONHandler.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Helper/OptionsMenu.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Helper/StringHandler.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Helper/Time.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Rendering/Environment.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Rendering/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Rendering/RenderObject.cpp
    ${CMAKE_SOURCE_DIR}/Tests/DsaDebug.cpp
    ${CMAKE_SOURCE_DIR}/Tests/TestEnv.cpp
    ${CMAKE_SOURCE_DIR}/Tests/_TEMPLATE_CLASS.cpp
)

#####################
# [OUTPUT]

message(STATUS "Setting targets...")

# Engine target
add_executable(Nebulite
    ${CMAKE_SOURCE_DIR}/Engine/main.cpp
    ${COMMON_SOURCES}
)
target_link_libraries(Nebulite PRIVATE 
    ${SDL2_PATH}/lib/libSDL2.a 
    ${SDL2_PATH}/lib/libSDL2_ttf.a
    ${SDL2_PATH}/lib/libSDL2_image.a
)
