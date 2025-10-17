############################################################
# Nebulite Build Configuration
#
# This CMake configuration uses a hybrid approach for dependencies:
# - SDL2, SDL2_ttf, and SDL2_image are prebuilt (DLLs for Windows, static for Linux)
#   and linked directly to avoid DLL/runtime issues and simplify cross-compilation.
# - Abseil is included as a subdirectory and built as part of the project,
#   as it works out of the box for both windows and linux.
#
# Key Directories:
# - All CMake build artifacts are placed in a dedicated build directory (e.g., build/windows-release).
# - Final binaries (executables, DLLs) are output to the build directory.
#
# Platform Notes:
# - On Linux, static SDL libraries are linked for portability.
# - On Windows (cross-compiled from Linux), prebuilt SDL DLL import libraries are linked.
#
# To add or update dependencies, see the external/ directory and install.sh script.
#
# For more details, see README.md.
############################################################

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

# Setting output inside ./build/<build-type>, 
# moving to proper location+renaming later on in the build.sh script
# CMAKE_BINARY_DIR is set by build.sh
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Echo the start of the configuration
message(STATUS "Starting configuration for project Nebulite")
message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Source dir: ${CMAKE_SOURCE_DIR}")
message(STATUS "Binary dir: ${CMAKE_BINARY_DIR}")

############################################################
# Shortcut paths to external dependencies
set(SDL2_BUILD_PATH     "${CMAKE_SOURCE_DIR}/.build/SDL2")
set(RAPIDJSON_PATH      "${CMAKE_SOURCE_DIR}/external/rapidjson")
set(EXPRTK_PATH         "${CMAKE_SOURCE_DIR}/external/exprtk")
set(TINYEXPR_PATH       "${CMAKE_SOURCE_DIR}/external/tinyexpr")
set(ABSEIL_PATH         "${CMAKE_SOURCE_DIR}/external/abseil")

############################################################
# Subdirectories
# Disable warnings for external dependencies
set(CMAKE_CXX_FLAGS_BACKUP ${CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
add_subdirectory(${ABSEIL_PATH})
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_BACKUP})

############################################################
# Source files - Automatically include all .cpp files in src/
file(GLOB_RECURSE COMMON_SOURCES 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# Adddition external files
list(APPEND COMMON_SOURCES 
    ${TINYEXPR_PATH}/tinyexpr.c
)

############################################################
# Define executable
add_executable(Nebulite
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${COMMON_SOURCES}
)

############################################################
# General include directories
target_include_directories(Nebulite
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    SYSTEM
        ${RAPIDJSON_PATH}/include/rapidjson
        ${EXPRTK_PATH}
        ${TINYEXPR_PATH}
        ${ABSEIL_PATH}
)

############################################################
# Compile options: Warnings
set_source_files_properties(
  ./src/ 
  "-w"
)

# Enable unused function detection (only for GCC/Clang)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(Nebulite PRIVATE
        -Wunused-function          # Warn about unused static functions
        -Wunused-variable          # Warn about unused variables (Deactivated for now, too many entries)
        -Wunused-parameter         # Warn about unused function parameters (Deactivated for now, too many entries)
        -Wunreachable-code         # Warn about unreachable code
        # Uncomment for more aggressive warnings:
        -Wall                   # Enable most common warnings
        -Wextra                 # Enable extra warnings
        #-Wpedantic              # Enable pedantic warnings (Not recommended due to __int128 conflicts in abseil libs)
    )
    
    # GCC-specific warnings
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(Nebulite PRIVATE
            -Wunused-but-set-variable  # Warn about variables that are set but never used
        )
    endif()
    
    # Clang-specific warnings
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(Nebulite PRIVATE
            -Wdead-code               # Warn about dead code (Clang only)
        )
    endif()
endif()

############################################################
# Platform-Specific

##################
# WINDOWS        #
##################
if(WIN32)
    message(STATUS "Targeting Windows")

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
        absl::flat_hash_map
        -lsetupapi
        -limm32
        -lversion
        -loleaut32
        -lole32
        -luuid
        -lwinmm
        -lws2_32
    )
##################
# APPLE          #
##################
elseif(APPLE)
    message(STATUS "Targeting macOS")

    set(SDL2_PATH        ${CMAKE_SOURCE_DIR}/external/SDL_Crossplatform_Local/external/SDL2)
    set(SDL2_TTF_PATH    ${CMAKE_SOURCE_DIR}/external/SDL_Crossplatform_Local/external/SDL_ttf)
    set(SDL2_IMAGE_PATH  ${CMAKE_SOURCE_DIR}/external/SDL_Crossplatform_Local/external/SDL_image)

    add_subdirectory(${SDL2_PATH})
    add_subdirectory(${SDL2_TTF_PATH})
    add_subdirectory(${SDL2_IMAGE_PATH})

    target_link_libraries(Nebulite PRIVATE SDL2 SDL2_ttf SDL2_image)

    # Frameworks needed for SDL2 on macOS
    target_link_libraries(Nebulite PRIVATE
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
            "-framework Carbon"
    )
##################
# LINUX          #
##################
elseif(UNIX)
    message(STATUS "Targeting Linux")

    target_include_directories(Nebulite PRIVATE
        ${SDL2_BUILD_PATH}/static_linux/include/SDL2
    )

    target_link_libraries(Nebulite PRIVATE
        "${SDL2_BUILD_PATH}/static_linux/lib/libSDL2main.a"
        "${SDL2_BUILD_PATH}/static_linux/lib/libSDL2_ttf.a"
        "${SDL2_BUILD_PATH}/static_linux/lib/libSDL2_image.a"
        "${SDL2_BUILD_PATH}/static_linux/lib/libSDL2.a"
        "${SDL2_BUILD_PATH}/static_linux/deps/freetype/lib/libfreetype.a"
        "${SDL2_BUILD_PATH}/static_linux/deps/harfbuzz/lib/libharfbuzz.a"
        absl::flat_hash_map
        # libfreetype depends on zlib, libpng, bzip2 and brotli
        z
        png
        bz2
        brotlidec
        m
    )
##################
# UNKNOWN        #
##################
else()
    message(STATUS "Targeting unknown platform... exiting.")

    # exit
    exit(1)
endif()