cmake_minimum_required(VERSION 3.16)
project(Nebulite LANGUAGES C CXX ASM_NASM)

############################################################
## Include CMake files
include(${CMAKE_SOURCE_DIR}/Tools/CMake/optimizations.cmake)
include(${CMAKE_SOURCE_DIR}/Tools/CMake/External/dependencies.cmake)
include(${CMAKE_SOURCE_DIR}/Tools/CMake/External/initmodules.cmake)
include(${CMAKE_SOURCE_DIR}/Tools/CMake/External/sdlsettings.cmake)
include(${CMAKE_SOURCE_DIR}/Tools/CMake/warnings.cmake)
include(${CMAKE_SOURCE_DIR}/Tools/CMake/binarysettings.cmake)
include(${CMAKE_SOURCE_DIR}/Tools/CMake/commonsources.cmake)

############################################################
# Info messages

# Echo the start of the configuration
message(STATUS "Starting configuration for project Nebulite")
message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Source dir: ${CMAKE_SOURCE_DIR}")
message(STATUS "Binary dir: ${CMAKE_BINARY_DIR}")

############################################################
## General Environment Setup

# Set C++ standard
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build artifacts go to ./tmp/build_<preset>/ via CMake presets,
# Final binaries are placed in ./bin/ with appropriate naming
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Setup assembler
enable_language(ASM)

# Fix LTO-related toolchain issues if needed
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries for external deps" FORCE)
lto_fix()

# Prepare SDL settings5a
sdl_setup()

############################################################
## Setup Project
initialize_git_submodules()
setup_external_subdirectories()
setup_binary_settings()
setup_common_sources(Nebulite)
configure_common_dependencies(Nebulite)
configure_warnings(Nebulite)
target_include_directories(Nebulite PUBLIC ${CMAKE_SOURCE_DIR}/include)

############################################################
# optimize
optimize_rapidjson(Nebulite)

############################################################
## Platform-Specific

##################
# WINDOWS        #
##################
if(WIN32)
    message(STATUS "Targeting Windows")
    
    # Include Windows-specific configuration
    include(${CMAKE_SOURCE_DIR}/Tools/CMake/Platforms/windbuild.cmake)
    
    # Configure for Windows
    configure_windows(Nebulite)

    # Prefer Win32 threads (avoid forcing pthread/winpthreads)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mthreads")
    set(CMAKE_THREAD_PREFER_PTHREAD FALSE)
    set(THREADS_PREFER_PTHREAD_FLAG FALSE)
    find_package(Threads REQUIRED)
##################
# APPLE          #
##################
elseif(APPLE)
    message(STATUS "Targeting macOS")
    
    # Include macOS-specific configuration
    include(${CMAKE_SOURCE_DIR}/Tools/CMake/Platforms/macbuild.cmake)
    
    # Configure for macOS
    configure_macos(Nebulite)
##################
# LINUX          #
##################
elseif(LINUX)
    message(STATUS "Targeting Linux")
    
    # Include Linux-specific configuration
    include(${CMAKE_SOURCE_DIR}/Tools/CMake/Platforms/linuxbuild.cmake)
    
    # Configure for Linux
    configure_linux(Nebulite)
##################
# UNKNOWN        #
##################
else()
    message(FATAL_ERROR "Unsupported platform")
endif()