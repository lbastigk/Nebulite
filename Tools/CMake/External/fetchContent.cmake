include(FetchContent)

function(fetchContent)
    ##########################################################
    # Fetch

    # Setup external location
    set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/external")

    # Avoid rebuilding dependencies every configure unless the tag changes.
    set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

    FetchContent_Declare(
            rapidjson
            GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
            GIT_TAG master    # Master is fine for rapidjson: rarely updated
    )
    FetchContent_GetProperties(rapidjson)
    if(NOT rapidjson_POPULATED)
        FetchContent_Populate(rapidjson)
    endif()
    set(rapidjson_SOURCE_DIR ${FETCHCONTENT_BASE_DIR}/rapidjson-src PARENT_SCOPE)

    FetchContent_Declare(
            tinyexpr
            GIT_REPOSITORY https://github.com/codeplea/tinyexpr.git
            GIT_TAG master    # Master is fine for tinyexpr: rarely updated
    )
    set(tinyexpr_SOURCE_DIR ${FETCHCONTENT_BASE_DIR}/tinyexpr-src PARENT_SCOPE)

    FetchContent_Declare(
            absl
            GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
            GIT_TAG lts_2026_05_26
    )
    # Absl sets source dir itself, so we don't need to set it here.

    FetchContent_Declare(
            SDL3
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG release-3.4.4
    )
    # SDL3 sets source dir itself, so we don't need to set it here.

    FetchContent_Declare(
            SDL3_image
            GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
            GIT_TAG release-3.4.4
    )
    # SDL3_image sets source dir itself, so we don't need to set it here.

    FetchContent_Declare(
            SDL3_ttf
            GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
            GIT_TAG release-3.2.2
    )
    # SDL3_ttf sets source dir itself, so we don't need to set it here.

    FetchContent_Declare(
            imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG v1.92.8
    )
    set(imgui_SOURCE_DIR ${FETCHCONTENT_BASE_DIR}/imgui-src PARENT_SCOPE)

    FetchContent_Declare(
            stb
            GIT_REPOSITORY https://github.com/nothings/stb.git
            GIT_TAG master
    )
    set(stb_SOURCE_DIR ${FETCHCONTENT_BASE_DIR}/stb-src PARENT_SCOPE)

    FetchContent_Declare(
            rmlui
            GIT_REPOSITORY https://github.com/mikke89/RmlUi.git
            GIT_TAG 6.2
    )
    # RmlUi sets source dir itself, so we don't need to set it here.

    ##########################################################
    # Settings: Pre
    set(ABSL_PROPAGATE_CXX_STD ON CACHE BOOL "" FORCE)
    set(ABSL_USE_SYSTEM_INCLUDES ON CACHE BOOL "" FORCE)
    set(ABSL_BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

    ##########################################################
    # Make available

    message(STATUS "Fetching external dependencies...")
    FetchContent_MakeAvailable(
            rapidjson
            tinyexpr
            absl
            SDL3
            SDL3_image
            SDL3_ttf
            imgui
            stb
            rmlui
    )

    ##########################################################
    # Settings: Post

    # RmlUi defaults to the freetype font engine and errors out if freetype isn't available.
    # Fall back to 'none' so configure can continue on toolchains without a freetype package.
    if(NOT TARGET Freetype::Freetype)
        find_package(Freetype QUIET)
    endif()

    if(NOT TARGET Freetype::Freetype)
        set(RMLUI_FONT_ENGINE "none" CACHE STRING "RmlUi font engine" FORCE)
        message(WARNING "Freetype not found. Setting RMLUI_FONT_ENGINE=none for this build. Install freetype or set Freetype_ROOT to enable RmlUi text rendering.")
    endif()
endfunction()

