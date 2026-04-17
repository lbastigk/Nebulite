############################################################
# Shortcut paths to external dependencies

# SDL3 official repositories
set(SDL3_PATH           "${CMAKE_SOURCE_DIR}/external/SDL3")
set(SDL3_TTF_PATH       "${CMAKE_SOURCE_DIR}/external/SDL3_ttf")
set(SDL3_IMAGE_PATH     "${CMAKE_SOURCE_DIR}/external/SDL3_image")

# GUI libraries
set(IMGUI_PATH          "${CMAKE_SOURCE_DIR}/external/imgui")
set(RMLUI_PATH          "${CMAKE_SOURCE_DIR}/external/RmlUi")

# Other external libraries
set(RAPIDJSON_PATH      "${CMAKE_SOURCE_DIR}/external/rapidjson")
set(TINYEXPR_PATH       "${CMAKE_SOURCE_DIR}/external/tinyexpr")
set(ABSEIL_PATH         "${CMAKE_SOURCE_DIR}/external/abseil")
set(STB_PATH            "${CMAKE_SOURCE_DIR}/external/stb")

############################################################
# Function to setup external subdirectories
function(setup_external_subdirectories)
    message(STATUS "Setting up external subdirectories...")
    add_subdirectory(${ABSEIL_PATH})
    add_subdirectory(${SDL3_PATH})
    add_subdirectory(${SDL3_TTF_PATH})
    add_subdirectory(${SDL3_IMAGE_PATH})

    # RmlUi defaults to the freetype font engine and errors out if freetype isn't available.
    # Fall back to 'none' so configure can continue on toolchains without a freetype package.
    if(NOT TARGET Freetype::Freetype)
        find_package(Freetype QUIET)
    endif()

    if(NOT TARGET Freetype::Freetype)
        set(RMLUI_FONT_ENGINE "none" CACHE STRING "RmlUi font engine" FORCE)
        message(WARNING "Freetype not found. Setting RMLUI_FONT_ENGINE=none for this build. Install freetype or set Freetype_ROOT to enable RmlUi text rendering.")
    endif()

    add_subdirectory(${RMLUI_PATH})
    message(STATUS "External subdirectories setup complete")
endfunction()

############################################################
# Function to configure common dependencies for a target
function(configure_common_dependencies target_name)
    message(STATUS "Configuring common dependencies for target: ${target_name}")

    add_compile_definitions(RMLUI_SDL_VERSION_MAJOR=3)

    # Include directories
    # normal include dir
    target_include_directories(${target_name}
            PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )

    # system includes (suppress warnings)
    target_include_directories(${target_name}
            SYSTEM PRIVATE
            ${RAPIDJSON_PATH}/include
            ${TINYEXPR_PATH}
            ${ABSEIL_PATH}
            ${SDL3_PATH}/include/
            ${SDL3_TTF_PATH}/include
            ${SDL3_IMAGE_PATH}/include
            ${IMGUI_PATH}
            ${IMGUI_PATH}/backends
            ${RMLUI_PATH}/Backends
            ${RMLUI_PATH}/Include
            ${STB_PATH}
    )

    # Setup imgui library
    add_library(imgui STATIC
            ${IMGUI_PATH}/imgui.cpp
            ${IMGUI_PATH}/imgui_draw.cpp
            ${IMGUI_PATH}/imgui_tables.cpp
            ${IMGUI_PATH}/imgui_widgets.cpp
            ${IMGUI_PATH}/imgui_demo.cpp     # optional
            ${IMGUI_PATH}/backends/imgui_impl_sdl3.cpp
            ${IMGUI_PATH}/backends/imgui_impl_sdlrenderer3.cpp
            ${IMGUI_PATH}/misc/cpp/imgui_stdlib.cpp
    )
    target_include_directories(imgui PUBLIC
            ${IMGUI_PATH}
            ${IMGUI_PATH}/backends
            ${SDL3_PATH}/include/
    )

    # Link libraries
    target_link_libraries(${target_name} PRIVATE
            absl::base
            absl::synchronization
            absl::strings
            absl::hash
            absl::flat_hash_map
            imgui
            RmlUi::RmlUi
            SDL3::SDL3
            SDL3_ttf::SDL3_ttf
            SDL3_image::SDL3_image
    )

    message(STATUS "Common dependencies configured for ${target_name}")
endfunction()