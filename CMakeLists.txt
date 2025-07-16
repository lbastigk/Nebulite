############################################################
# General
cmake_minimum_required(VERSION 3.16)
project(Nebulite)

add_definitions(-DSDL_DISABLE_IMPLEMENTS)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-g -w)

# Echo the start of the configuration
message(STATUS "Starting configuration for project Nebulite")

# Detect platform
if(WIN32)
    set(IS_WINDOWS TRUE)
else()
    set(IS_WINDOWS FALSE)
endif()

# Set binary output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Application/bin)
message(STATUS "Binary output directory set to: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

############################################################
# Paths to external libs
set(SDL2_BUILD_PATH "${CMAKE_SOURCE_DIR}/external/SDL2_build")
set(RAPIDJSON_PATH  "${CMAKE_SOURCE_DIR}/external/rapidjson")
set(EXPRTK_PATH     "${CMAKE_SOURCE_DIR}/external/exprtk")
set(TINYEXPR_PATH   "${CMAKE_SOURCE_DIR}/external/tinyexpr")
set(ABSEIL_PATH     "${CMAKE_SOURCE_DIR}/external/abseil")

# Add abseil subdirectory
add_subdirectory(${ABSEIL_PATH} ${CMAKE_BINARY_DIR}/external/abseil_build)

############################################################
# General include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src/Engine
    ${CMAKE_SOURCE_DIR}/src/Engine/Data
    ${CMAKE_SOURCE_DIR}/src/Engine/FuncTrees
    ${CMAKE_SOURCE_DIR}/src/Engine/Rendering
    ${CMAKE_SOURCE_DIR}/src/Engine/Helper
    ${CMAKE_SOURCE_DIR}/src/Platform
    ${CMAKE_SOURCE_DIR}/src/Editor
    ${CMAKE_SOURCE_DIR}/src/Tests
    ${RAPIDJSON_PATH}/include/rapidjson
    ${EXPRTK_PATH}
    ${TINYEXPR_PATH}
    ${ABSEIL_PATH}
)

############################################################
# Source files
set(COMMON_SOURCES
    ${TINYEXPR_PATH}/tinyexpr.c
    ${CMAKE_SOURCE_DIR}/src/Platform/Platform.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Nebulite.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Data/JSON.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/FuncTrees/MainFuncTree.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Helper/FileManagement.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Helper/StringHandler.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Helper/Time.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Rendering/Environment.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Rendering/Invoke.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Rendering/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Rendering/RenderObject.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Rendering/RenderObjectContainer.cpp
)

############################################################
# Define executable
add_executable(Nebulite
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${COMMON_SOURCES}
)


############################################################
# Link libraries
target_link_libraries(Nebulite PRIVATE
    absl::flat_hash_map
    absl::base
    absl::strings
)

############################################################
# Platform-Specific
if(IS_WINDOWS)
    message(STATUS "Targeting Windows: linking SDL static libraries")
    set(CMAKE_SYSTEM_NAME Windows)
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wuninitialized)

    # Include SDL Libraries
    target_include_directories(Nebulite PRIVATE
        ${SDL2_BUILD_PATH}/shared_windows/include/SDL2
    )

    # Force console window in Debug, GUI (no console) in Release
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_options(Nebulite PRIVATE -mconsole)
    else()
        # Later on, for actual release:
        #target_link_options(Nebulite PRIVATE -mwindows)

        # For now, for console output in release version:
        target_link_options(Nebulite PRIVATE -mconsole)
    endif()

    # Link libraries
    target_link_libraries(Nebulite PRIVATE
        -lmingw32
        "${SDL2_BUILD_PATH}/shared_windows/lib/libSDL2main.a"
        "${SDL2_BUILD_PATH}/shared_windows/lib/libSDL2_ttf.dll.a"
        "${SDL2_BUILD_PATH}/shared_windows/lib/libSDL2_image.dll.a"
        "${SDL2_BUILD_PATH}/shared_windows/lib/libSDL2.dll.a"
        -lsetupapi
        -limm32
        -lversion
        -loleaut32
        -lole32
        -luuid
        -lwinmm
        -lws2_32
    )
else()
    message(STATUS "Targeting Linux: linking SDL static libraries")

    target_include_directories(Nebulite PRIVATE
        ${SDL2_BUILD_PATH}/shared/include/SDL2
    )

    target_link_libraries(Nebulite PRIVATE
        "${SDL2_BUILD_PATH}/static/lib/libSDL2main.a"
        "${SDL2_BUILD_PATH}/static/lib/libSDL2_ttf.a"
        "${SDL2_BUILD_PATH}/static/lib/libSDL2_image.a"
        "${SDL2_BUILD_PATH}/static/lib/libSDL2.a"
    )
endif()

