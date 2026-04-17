function(sdl_setup)
    set(SDLTTF_VENDORED ON CACHE BOOL "Use vendored libraries for SDL3_ttf" FORCE)
    set(SDLIMAGE_VENDORED OFF CACHE BOOL "Use vendored libraries for SDL3_image" FORCE)

    # Required, otherwise SDL3_image does not find libpng.a
    set(CMAKE_FIND_PACKAGE_PREFER_CONFIG OFF CACHE BOOL "" FORCE)
endfunction()
