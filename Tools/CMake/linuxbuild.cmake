# Linux-specific build configuration
# This file contains Linux-specific CMake configuration

if(NOT UNIX OR APPLE)
    message(WARNING "linuxbuild.cmake is intended for Linux builds only")
    return()
endif()

message(STATUS "Loading Linux build configuration...")

# Linux-specific compile definitions (if any)
# add_compile_definitions(LINUX_SPECIFIC_FLAG)

# Linux-specific SDL2 configuration
function(configure_linux_sdl2 target_name)
    message(STATUS "Configuring SDL2 for Linux target: ${target_name}")
    
    # Configure SDL2 build options to minimize dependencies
    set(SDL_SHARED OFF CACHE BOOL "Build SDL2 as shared library" FORCE)
    set(SDL_STATIC ON CACHE BOOL "Build SDL2 as static library" FORCE)
    set(SDL_TEST OFF CACHE BOOL "Build SDL2 test programs" FORCE)
    
    # Configure SDL2_image build options 
    set(SDL2IMAGE_INSTALL OFF CACHE BOOL "Disable SDL2_image install" FORCE)
    set(SDL2IMAGE_SAMPLES OFF CACHE BOOL "Disable SDL2_image samples" FORCE)
    set(SDL2IMAGE_TESTS OFF CACHE BOOL "Disable SDL2_image tests" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
    
    # Configure SDL2_ttf build options
    set(SDL2TTF_INSTALL OFF CACHE BOOL "Disable SDL2_ttf install" FORCE)
    set(SDL2TTF_SAMPLES OFF CACHE BOOL "Disable SDL2_ttf samples" FORCE)
    
    # Add SDL2 subdirectories in correct order (SDL2 first, then extensions)
    add_subdirectory(${SDL2_PATH} SDL2 EXCLUDE_FROM_ALL)
    add_subdirectory(${SDL2_TTF_PATH} SDL2_ttf EXCLUDE_FROM_ALL)
    add_subdirectory(${SDL2_IMAGE_PATH} SDL2_image EXCLUDE_FROM_ALL)
    
    # Link SDL2 libraries using the correct target names
    target_link_libraries(${target_name} PRIVATE
        SDL2::SDL2main
        SDL2::SDL2-static
        SDL2_ttf
        SDL2_image
        absl::flat_hash_map
    )
    
    # Additional system libraries needed for static linking on Linux
    target_link_libraries(${target_name} PRIVATE
        m           # Math library
        dl          # Dynamic loading
        pthread     # Threading
    )
    
    message(STATUS "SDL2 configured for Linux")
endfunction()

message(STATUS "Linux build configuration loaded successfully")