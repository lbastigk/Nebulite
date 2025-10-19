# macOS-specific build configuration
# This file contains macOS-specific CMake configuration

if(NOT APPLE)
    message(WARNING "macbuild.cmake is intended for macOS builds only")
    return()
endif()

message(STATUS "Loading macOS build configuration...")

# macOS-specific compile definitions (if any)
# add_compile_definitions(MACOS_SPECIFIC_FLAG)

# macOS-specific SDL2 configuration
function(configure_macos_sdl2 target_name)
    message(STATUS "Configuring SDL2 for macOS target: ${target_name}")
    
    # Force static linking to avoid shared library dependencies
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
    message(STATUS "macOS build: Forced static linking (BUILD_SHARED_LIBS=OFF)")
    
    # Configure SDL2 build options to minimize dependencies
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
    set(SDL2TTF_VENDORED ON CACHE BOOL "Use bundled freetype for macOS" FORCE)
    
    # Add SDL2 subdirectories in correct order (SDL2 first, then extensions)
    add_subdirectory(${SDL2_PATH} SDL2 EXCLUDE_FROM_ALL)
    add_subdirectory(${SDL2_TTF_PATH} SDL2_ttf EXCLUDE_FROM_ALL)
    add_subdirectory(${SDL2_IMAGE_PATH} SDL2_image EXCLUDE_FROM_ALL)
    
    # Use the same SDL2 submodules as Linux/Windows
    message(STATUS "Using SDL2 submodules for macOS cross-compilation")
    
    # Link against the static SDL2 libraries built from submodules
    target_link_libraries(${target_name} PRIVATE
        SDL2main
        SDL2-static
        SDL2_ttf
        SDL2_image
        absl::flat_hash_map
    )

    # Frameworks needed for SDL2 on macOS
    target_link_libraries(${target_name} PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework Carbon"
        "-framework AudioToolbox"
        "-framework CoreAudio"
        "-framework ForceFeedback"
    )
    
    message(STATUS "SDL2 configured for macOS")
endfunction()

message(STATUS "macOS build configuration loaded successfully")