############################################################
# Function to configure common dependencies for a target
function(configure_common_dependencies target_name)
    message(STATUS "Configuring common dependencies for target: ${target_name}")

    # Set SDL version macro for RmlUi
    target_compile_definitions(${target_name}
            PRIVATE
            RMLUI_SDL_VERSION_MAJOR=3
    )

    # Include directories
    # normal include dir
    target_include_directories(${target_name}
            PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )

    # system includes (suppress warnings)
    target_include_directories(${target_name}
            SYSTEM PRIVATE
            ${rapidjson_SOURCE_DIR}/include
            ${tinyexpr_SOURCE_DIR}
            ${SDL3_SOURCE_DIR}/include
            ${SDL3_TTF_SOURCE_DIR}/include
            ${SDL3_IMAGE_SOURCE_DIR}/include
            ${imgui_SOURCE_DIR}
            ${imgui_SOURCE_DIR}/backends
            ${RmlUi_SOURCE_DIR}/Backends
            ${RmlUi_SOURCE_DIR}/Include
            ${RmlUi_SOURCE_DIR}/Source
            ${stb_SOURCE_DIR}
    )

    # Setup imgui library
    if(NOT TARGET imgui)
        add_library(imgui STATIC
                ${imgui_SOURCE_DIR}/imgui.cpp
                ${imgui_SOURCE_DIR}/imgui_draw.cpp
                ${imgui_SOURCE_DIR}/imgui_tables.cpp
                ${imgui_SOURCE_DIR}/imgui_widgets.cpp
                ${imgui_SOURCE_DIR}/imgui_demo.cpp     # optional
                ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
                ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
                ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
        )
    endif()

    # Add include directories for imgui
    target_include_directories(imgui PUBLIC
            ${imgui_SOURCE_DIR}
            ${imgui_SOURCE_DIR}/backends
            ${SDL3_SOURCE_DIR}/include
    )

    # Link libraries
    target_link_libraries(${target_name} PRIVATE
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