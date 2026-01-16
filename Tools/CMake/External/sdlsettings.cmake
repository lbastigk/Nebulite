function(sdl_setup)
    set(SDLTTF_VENDORED ON CACHE BOOL "Use vendored libraries for SDL3_ttf" FORCE)

    # TODO: This breaks at the moment, needs investigation
    #set(CMAKE_FIND_PACKAGE_PREFER_CONFIG OFF CACHE BOOL "Prefer config mode for SDL3_image" FORCE)
    #set(SDLIMAGE_VENDORED ON CACHE BOOL "Use vendored libraries for SDL3_image" FORCE)
endfunction()
