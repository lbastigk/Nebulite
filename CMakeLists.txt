############################################################
## General
cmake_minimum_required(VERSION 3.16)
project(Nebulite)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build artifacts go to ./tmp/build_<preset>/ via CMake presets,
# Final binaries are placed in ./bin/ with appropriate naming
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Echo the start of the configuration
message(STATUS "Starting configuration for project Nebulite")
message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Source dir: ${CMAKE_SOURCE_DIR}")
message(STATUS "Binary dir: ${CMAKE_BINARY_DIR}")

############################################################
## Include CMake files
include(${CMAKE_SOURCE_DIR}/Tools/CMake/External/dependencies.cmake)
include(${CMAKE_SOURCE_DIR}/Tools/CMake/External/initmodules.cmake)
include(${CMAKE_SOURCE_DIR}/Tools/CMake/External/sdlsettings.cmake)
include(${CMAKE_SOURCE_DIR}/Tools/CMake/warnings.cmake)
include(${CMAKE_SOURCE_DIR}/Tools/CMake/binarysettings.cmake)
include(${CMAKE_SOURCE_DIR}/Tools/CMake/commonsources.cmake)

############################################################
## Git submodule initialization and update

# - initializes and updates git submodules
# - recursively updates nested submodules
initialize_git_submodules()

############################################################
## Setup external dependencies
setup_external_subdirectories()

############################################################
## Source files
setup_common_sources()

############################################################
## Define executable
setup_binary_settings()

############################################################
## Dependencies
configure_common_dependencies(Nebulite)

############################################################
## Warnings

# - sets up common warnings
# - sets up compilter-specific warnings
# - sets up build-type-specific warnings
configure_warnings(Nebulite)

############################################################
## Platform-Specific

##################
# WINDOWS        #
##################
if(WIN32)
    message(STATUS "Targeting Windows")
    
    # Include Windows-specific configuration
    include(${CMAKE_SOURCE_DIR}/Tools/CMake/Platforms/windbuild.cmake)
    
    # Configure SDL2 for Windows
    configure_windows_sdl2(Nebulite)
##################
# APPLE          #
##################
elseif(APPLE)
    message(STATUS "Targeting macOS")
    
    # Include macOS-specific configuration
    include(${CMAKE_SOURCE_DIR}/Tools/CMake/Platforms/macbuild.cmake)
    
    # Configure SDL2 for macOS
    configure_macos(Nebulite)
##################
# LINUX          #
##################
elseif(UNIX)
    message(STATUS "Targeting Linux")
    
    # Include Linux-specific configuration
    include(${CMAKE_SOURCE_DIR}/Tools/CMake/Platforms/linuxbuild.cmake)
    
    # Configure SDL2 for Linux
    configure_linux(Nebulite)
##################
# UNKNOWN        #
##################
else()
    message(STATUS "Targeting unknown platform... exiting.")

    # exit
    exit(1)
endif()