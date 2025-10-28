function(sdl_setup)
    # Shared SDL2/SDL2_image/SDL2_ttf build option settings

    # Define global compile definitions (prefer modern CMake)
    add_compile_definitions(SDL_DISABLE_IMPLEMENTS)

    # SDL2 build options
    set(SDL_SHARED OFF CACHE BOOL "Disable building SDL2 as shared library" FORCE)
    set(SDL_STATIC ON  CACHE BOOL "Build SDL2 as static library" FORCE)
    set(SDL_TEST   OFF CACHE BOOL "Disable building SDL2 test programs" FORCE)

    # Disable building shared libraries globally
    set(SDL_SHARED OFF CACHE BOOL "Disable building SDL2 as shared library" FORCE)
    set(SDL2TTF_SHARED OFF CACHE BOOL "Disable building SDL2_ttf as shared library" FORCE)
    set(SDL2IMAGE_SHARED OFF CACHE BOOL "Disable building SDL2_image as shared library" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Disable building shared libraries" FORCE)
endfunction()