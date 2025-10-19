# Windows-specific build configuration
# This file contains Windows-specific CMake configuration that can be included
# in the main CMakeLists.txt file

if(NOT WIN32)
    message(WARNING "windbuild.cmake is intended for Windows builds only")
    return()
endif()

message(STATUS "Loading Windows build configuration...")

# Windows-specific compile definitions
add_compile_definitions(WIN32_LEAN_AND_MEAN)
add_compile_definitions(NOMINMAX)

# Windows-specific SDL2 configuration using SDL2 submodules (same as Linux)
function(configure_windows_sdl2 target_name)
    message(STATUS "Configuring SDL2 for Windows target: ${target_name}")
    
    # Force static linking to avoid DLL dependencies
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
    message(STATUS "Windows build: Forced static linking (BUILD_SHARED_LIBS=OFF)")
    
    # Configure SDL2 build options to minimize dependencies (same as Linux)
    set(SDL_SHARED OFF CACHE BOOL "Build SDL2 as shared library" FORCE)
    set(SDL_STATIC ON CACHE BOOL "Build SDL2 as static library" FORCE)
    set(SDL_TEST OFF CACHE BOOL "Build SDL2 test programs" FORCE)
    
    # Configure SDL2_image build options 
    set(SDL2IMAGE_INSTALL OFF CACHE BOOL "Disable SDL2_image install" FORCE)
    set(SDL2IMAGE_SAMPLES OFF CACHE BOOL "Disable SDL2_image samples" FORCE)
    set(SDL2IMAGE_TESTS OFF CACHE BOOL "Disable SDL2_image tests" FORCE)
    
    # Configure SDL2_ttf build options
    set(SDL2TTF_INSTALL OFF CACHE BOOL "Disable SDL2_ttf install" FORCE)
    set(SDL2TTF_SAMPLES OFF CACHE BOOL "Disable SDL2_ttf samples" FORCE)
    set(SDL2TTF_VENDORED ON CACHE BOOL "Use bundled freetype for Windows" FORCE)
    
    # Add SDL2 subdirectories in correct order (SDL2 first, then extensions)
    add_subdirectory(${SDL2_PATH} SDL2 EXCLUDE_FROM_ALL)
    add_subdirectory(${SDL2_TTF_PATH} SDL2_ttf EXCLUDE_FROM_ALL)
    add_subdirectory(${SDL2_IMAGE_PATH} SDL2_image EXCLUDE_FROM_ALL)
    
    # Use the same SDL2 submodules as Linux
    message(STATUS "Using SDL2 submodules for Windows cross-compilation")
    
    # Link against the static SDL2 libraries built from submodules
    target_link_libraries(${target_name} PRIVATE
        SDL2main
        SDL2-static
        SDL2_ttf
        SDL2_image
        absl::flat_hash_map
    )
    
    # Windows system libraries needed for static linking
    target_link_libraries(${target_name} PRIVATE
        ws2_32      # Winsock
        winmm       # Windows multimedia
        setupapi    # Setup API
        version     # Version info
    )

    # Static linking flags to avoid DLL dependencies
    target_link_options(${target_name} PRIVATE
        -static-libgcc
        -static-libstdc++
        -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive
    )
    message(STATUS "Windows build: Static linking enabled (no DLL dependencies)")

    # Force console window in Debug, GUI (no console) in Release
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_options(${target_name} PRIVATE -mconsole)
        message(STATUS "Windows Debug build: Console enabled")
    else()
        # For release builds - currently keeping console for output
        target_link_options(${target_name} PRIVATE -mconsole)
        message(STATUS "Windows Release build: Console enabled (for debugging)")
        
        # Uncomment for true GUI release (no console):
        # target_link_options(${target_name} PRIVATE -mwindows)
    endif()

    # Link Windows-specific system libraries
    target_link_libraries(${target_name} PRIVATE
        -lmingw32
        -lsetupapi
        -limm32
        -lversion
        -loleaut32
        -lole32
        -luuid
        -lwinmm
        -lws2_32
    )
    
    message(STATUS "SDL2 configured for Windows")
endfunction()

message(STATUS "Windows build configuration loaded successfully")