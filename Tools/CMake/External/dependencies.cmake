############################################################
# Function to configure common dependencies for a target
function(configure_common_dependencies target_name)
    message(STATUS "Configuring common dependencies for target: ${target_name}")

    ###############################
    # Compile definitions

    # Set SDL version macro for RmlUi
    target_compile_definitions(${target_name} PRIVATE
            RMLUI_SDL_VERSION_MAJOR=3
    )

    ###############################
    # Includes

    # Include directories
    # normal include dir
    target_include_directories(${target_name} PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )

    # system includes (suppress warnings)
    target_include_directories(${target_name} SYSTEM PRIVATE
            ${rapidjson_SOURCE_DIR}/include
            ${tinyexpr_SOURCE_DIR}
            ${SDL3_SOURCE_DIR}/include
            ${SDL3_TTF_SOURCE_DIR}/include
            ${SDL3_IMAGE_SOURCE_DIR}/include
            ${imgui_SOURCE_DIR}
            ${implot_SOURCE_DIR}
            ${imgui_SOURCE_DIR}/backends
            ${RmlUi_SOURCE_DIR}/Backends
            ${RmlUi_SOURCE_DIR}/Include
            ${RmlUi_SOURCE_DIR}/Source
            ${stb_SOURCE_DIR}
    )

    ###############################
    # Setup libraries

    # Setup imgui library
    if(NOT TARGET imgui)
        # Sources
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

        # Includes
        target_include_directories(imgui PUBLIC
                ${imgui_SOURCE_DIR}
                ${imgui_SOURCE_DIR}/backends
                ${SDL3_SOURCE_DIR}/include
        )
    endif()

    # Setup implot library
    if(NOT TARGET implot)
        # Sources
        add_library(implot STATIC
                ${implot_SOURCE_DIR}/implot.cpp
                ${implot_SOURCE_DIR}/implot_items.cpp
                ${implot_SOURCE_DIR}/implot_demo.cpp     # optional
        )

        # Includes
        target_include_directories(implot PUBLIC
                ${implot_SOURCE_DIR}
                ${imgui_SOURCE_DIR}
        )

        # Linkage
        target_link_libraries(implot PUBLIC imgui)
    endif()

    ###############################
    # Link libraries
    target_link_libraries(${target_name} PRIVATE
            absl::hash
            absl::flat_hash_map
            imgui
            implot
            RmlUi::RmlUi
            SDL3::SDL3
            SDL3_ttf::SDL3_ttf
            SDL3_image::SDL3_image
    )

    message(STATUS "Common dependencies configured for ${target_name}")
endfunction()