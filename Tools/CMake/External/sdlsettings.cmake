function(sdl_setup)
    add_compile_definitions(SDL_DISABLE_IMPLEMENTS)

    # Determine canonical SDL2_SHARED (prefer preset/CLI if provided)
    if(NOT DEFINED SDL2_SHARED)
        if(DEFINED SDL_SHARED)
            set(SDL2_SHARED ${SDL_SHARED})
        else()
            set(SDL2_SHARED OFF)
        endif()
        set(SDL2_SHARED ${SDL2_SHARED} CACHE BOOL "Build SDL2 as shared library")
    endif()

    # Make alternate names consistent with the canonical value (force to avoid mismatches)
    set(SDL_SHARED ${SDL2_SHARED} CACHE BOOL "Alias for SDL2_SHARED" FORCE)
    set(SDL2TTF_SHARED ${SDL2_SHARED} CACHE BOOL "Build SDL2_ttf as shared library" FORCE)
    set(SDL2_TTF_SHARED ${SDL2_SHARED} CACHE BOOL "Alias for SDL2_ttf shared" FORCE)
    set(SDL2IMAGE_SHARED ${SDL2_SHARED} CACHE BOOL "Build SDL2_image as shared library" FORCE)

    # Disable building shared libs globally to avoid surprises
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Disable building shared libraries" FORCE)

    # Other SDL options (respect presets/CLI when present)
    if(NOT DEFINED SDL_STATIC)
        set(SDL_STATIC ON CACHE BOOL "Build SDL as static library")
    endif()

    if(NOT DEFINED SDL_TEST)
        set(SDL_TEST OFF CACHE BOOL "Disable building SDL test programs")
    endif()
endfunction()
