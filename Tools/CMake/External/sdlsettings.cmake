function(sdl_setup)
    add_compile_definitions(SDL_DISABLE_IMPLEMENTS)

    # Determine canonical SDL3_SHARED (prefer preset/CLI if provided)
    if(NOT DEFINED SDL3_SHARED)
        if(DEFINED SDL_SHARED)
            set(SDL3_SHARED ${SDL_SHARED})
        else()
            set(SDL3_SHARED OFF)
        endif()
        set(SDL3_SHARED ${SDL3_SHARED} CACHE BOOL "Build SDL3 as shared library")
    endif()

    # Make alternate names consistent with the canonical value (but don't FORCE)
    # Allow external projects or toolchain to override individual components.
    if(NOT DEFINED SDL_SHARED)
        set(SDL_SHARED ${SDL3_SHARED} CACHE BOOL "Alias for SDL3_SHARED")
    endif()
    if(NOT DEFINED SDL3_TTF_SHARED)
        set(SDL3_TTF_SHARED ${SDL3_SHARED} CACHE BOOL "Build SDL3_ttf as shared library")
    endif()
    if(NOT DEFINED SDL3IMAGE_SHARED)
        set(SDL3IMAGE_SHARED ${SDL3_SHARED} CACHE BOOL "Build SDL3_image as shared library")
    endif()

    # Do not unconditionally disable shared libs globally; only set a default if not set
    if(NOT DEFINED BUILD_SHARED_LIBS)
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "Disable building shared libraries")
    endif()

    # Other SDL options (respect presets/CLI when present)
    if(NOT DEFINED SDL_STATIC)
        set(SDL_STATIC ON CACHE BOOL "Build SDL as static library")
    endif()

    if(NOT DEFINED SDL_TEST)
        set(SDL_TEST OFF CACHE BOOL "Disable building SDL test programs")
    endif()
endfunction()