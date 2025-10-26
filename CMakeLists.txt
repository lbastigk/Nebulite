############################################################
# General
cmake_minimum_required(VERSION 3.16)
project(Nebulite)

# Define global compile definitions (prefer modern CMake)
add_compile_definitions(SDL_DISABLE_IMPLEMENTS)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Only add debug info in Debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g)
endif()

# Coverage build configuration
if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
    add_compile_options(-g --coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
    message(STATUS "Coverage build enabled")
endif()

# Build artifacts go to ./tmp/build_<preset>/ via CMake presets,
# Final binaries are placed in ./bin/ with appropriate naming
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Echo the start of the configuration
message(STATUS "Starting configuration for project Nebulite")
message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Source dir: ${CMAKE_SOURCE_DIR}")
message(STATUS "Binary dir: ${CMAKE_BINARY_DIR}")

############################################################
# Git submodule initialization and update
message(STATUS "Updating git submodules...")
execute_process(
    COMMAND git submodule update --init --recursive
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE GIT_SUBMOD_RESULT
    OUTPUT_QUIET
    ERROR_QUIET
)

if(NOT GIT_SUBMOD_RESULT EQUAL "0")
    message(WARNING "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}")
else()
    message(STATUS "Git submodules updated successfully")
endif()

############################################################
# Include modular CMake configurations from Tools/CMake directory
include(${CMAKE_SOURCE_DIR}/Tools/CMake/warnings.cmake)
include(${CMAKE_SOURCE_DIR}/Tools/CMake/dependencies.cmake)

############################################################
# Setup external dependencies
setup_external_subdirectories()

############################################################
# Source files - Automatically include all .cpp files in src/
file(GLOB_RECURSE COMMON_SOURCES 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# Additional external files
list(APPEND COMMON_SOURCES 
    ${TINYEXPR_PATH}/tinyexpr.c
)

############################################################
# Define executable
add_executable(Nebulite
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${COMMON_SOURCES}
)
target_compile_options(Nebulite PRIVATE -Wno-system-headers) # Suppress warnings from system headers

############################################################
# Configure binary output directory and naming
set_target_properties(Nebulite PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin
)

# Set output name based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(Nebulite PROPERTIES OUTPUT_NAME "Nebulite_Debug")
elseif(CMAKE_BUILD_TYPE STREQUAL "Coverage")
    set_target_properties(Nebulite PROPERTIES OUTPUT_NAME "Nebulite_Coverage")
else()
    set_target_properties(Nebulite PROPERTIES OUTPUT_NAME "Nebulite")
endif()

############################################################
# Configure common dependencies and warnings
configure_common_dependencies(Nebulite)
configure_warnings(Nebulite)

############################################################
# Platform-Specific

##################
# WINDOWS        #
##################
if(WIN32)
    message(STATUS "Targeting Windows")
    
    # Include Windows-specific configuration
    include(${CMAKE_SOURCE_DIR}/Tools/CMake/windbuild.cmake)
    
    # Configure SDL2 for Windows
    configure_windows_sdl2(Nebulite)
##################
# APPLE          #
##################
elseif(APPLE)
    message(STATUS "Targeting macOS")
    
    # Include macOS-specific configuration
    include(${CMAKE_SOURCE_DIR}/Tools/CMake/macbuild.cmake)
    
    # Configure SDL2 for macOS
    configure_macos_sdl2(Nebulite)
##################
# LINUX          #
##################
elseif(UNIX)
    message(STATUS "Targeting Linux")
    
    # Include Linux-specific configuration
    include(${CMAKE_SOURCE_DIR}/Tools/CMake/linuxbuild.cmake)
    
    # Configure SDL2 for Linux
    configure_linux_sdl2(Nebulite)
##################
# UNKNOWN        #
##################
else()
    message(STATUS "Targeting unknown platform... exiting.")

    # exit
    exit(1)
endif()