# Dependencies configuration
message(STATUS "Loading dependencies configuration...")

############################################################
# Options: prefer system packages instead of building vendored copies
option(USE_SYSTEM_ABSEIL "Use system-installed Abseil instead of vendored ${ABSEIL_PATH}" OFF)
option(USE_SYSTEM_SDL2 "Use system-installed SDL2 instead of vendored ${SDL2_PATH}" OFF)
option(USE_SYSTEM_SDL2_TTF "Use system-installed SDL2_ttf instead of vendored ${SDL2_TTF_PATH}" OFF)
option(USE_SYSTEM_SDL2_IMAGE "Use system-installed SDL2_image instead of vendored ${SDL2_IMAGE_PATH}" OFF)

############################################################
# Shortcut paths to external dependencies
set(RAPIDJSON_PATH      "${CMAKE_SOURCE_DIR}/external/rapidjson")
set(EXPRTK_PATH         "${CMAKE_SOURCE_DIR}/external/exprtk")
set(TINYEXPR_PATH       "${CMAKE_SOURCE_DIR}/external/tinyexpr")
set(ABSEIL_PATH         "${CMAKE_SOURCE_DIR}/external/abseil")

# SDL2 official repositories
set(SDL2_PATH           "${CMAKE_SOURCE_DIR}/external/SDL2")
set(SDL2_TTF_PATH       "${CMAKE_SOURCE_DIR}/external/SDL2_ttf")
set(SDL2_IMAGE_PATH     "${CMAKE_SOURCE_DIR}/external/SDL2_image")

############################################################
# Try to prefer system packages (quietly)
if(USE_SYSTEM_ABSEIL)
    find_package(absl CONFIG QUIET COMPONENTS container)
endif()

if(USE_SYSTEM_SDL2)
    find_package(SDL2 CONFIG QUIET)
endif()

if(USE_SYSTEM_SDL2_TTF)
    find_package(SDL2_ttf CONFIG QUIET)
endif()

if(USE_SYSTEM_SDL2_IMAGE)
    find_package(SDL2_image CONFIG QUIET)
endif()

find_package(Freetype QUIET)  # SDL2_ttf/freetype interplay handled by skipping vendored ttf when possible

############################################################
# Linked libraries from all external dependencies

set(SDL2_LINK_LIBS
        SDL2::SDL2main
        SDL2::SDL2-static
        SDL2_ttf
        SDL2_image
)

set(ABSL_LINK_LIBS
        absl::flat_hash_map
)

############################################################
# Function to configure common dependencies for a target
function(configure_common_dependencies target_name)
    message(STATUS "Configuring common dependencies for target: ${target_name}")

    # Include directories (always add project include)
    target_include_directories(${target_name}
            PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )

    # Add third-party include search paths only when not provided by system targets.
    # RapidJSON/exprtk/tinyexpr are header bundles, keep them as SYSTEM includes.
    target_include_directories(${target_name} SYSTEM PRIVATE
            ${RAPIDJSON_PATH}/include
            ${EXPRTK_PATH}
            ${TINYEXPR_PATH}
    )

    # Abseil headers: if system absl target is available use its INTERFACE dirs,
    # otherwise, if vendored path exists add vendored path as include.
    if(TARGET absl::flat_hash_map)
        message(VERBOSE "Using Abseil target absl::flat_hash_map")
        target_include_directories(${target_name} SYSTEM PRIVATE
                $<TARGET_PROPERTY:absl::flat_hash_map,INTERFACE_INCLUDE_DIRECTORIES>
        )
    elseif(EXISTS "${ABSEIL_PATH}")
        message(VERBOSE "Using vendored Abseil headers from ${ABSEIL_PATH}")
        target_include_directories(${target_name} SYSTEM PRIVATE "${ABSEIL_PATH}")
    endif()

    # Link common libraries (link only targets that exist)
    foreach(_lib ${SDL2_LINK_LIBS})
        if(TARGET ${_lib})
            target_link_libraries(${target_name} PRIVATE ${_lib})
        else()
            message(VERBOSE "Link target ${_lib} not available; skipping")
        endif()
    endforeach()

    foreach(_lib ${ABSL_LINK_LIBS})
        if(TARGET ${_lib})
            target_link_libraries(${target_name} PRIVATE ${_lib})
        else()
            message(VERBOSE "Abseil link target ${_lib} not available; assuming header-only or system include-only usage")
        endif()
    endforeach()

    ##########################################################
    # Suppress warnings for bundled C sources (tinyexpr)
    if(EXISTS "${TINYEXPR_PATH}/tinyexpr.c")
        message(STATUS "Applying per-source warning suppression for tinyexpr.c")
        set_source_files_properties("${TINYEXPR_PATH}/tinyexpr.c" PROPERTIES
                COMPILE_OPTIONS "$<$<OR:$<COMPILE_LANG_AND_ID:C,Clang>,$<COMPILE_LANG_AND_ID:C,GNU>>:-w>;$<$<COMPILE_LANG_AND_ID:C,MSVC>:/W0>"
        )
    endif()

    message(STATUS "Common dependencies configured for ${target_name}")
endfunction()

############################################################
# Function to setup external subdirectories (call once)
function(setup_external_subdirectories)
    message(STATUS "Setting up external subdirectories...")

    # Abseil: skip vendored add_subdirectory if system target available
    if(TARGET absl::flat_hash_map)
        message(STATUS "Abseil system target present; skipping vendored add_subdirectory")
    else()
        if(EXISTS "${ABSEIL_PATH}/CMakeLists.txt")
            message(STATUS "Adding vendored Abseil from ${ABSEIL_PATH}")
            add_subdirectory(${ABSEIL_PATH})
        else()
            message(VERBOSE "Vendored Abseil not found at ${ABSEIL_PATH}")
        endif()
    endif()

    # Setup Environment variables for SDL2 builds
    sdl_setup()

    # SDL2 and its extensions: prefer system targets if found, otherwise add vendored
    if(TARGET SDL2::SDL2 OR TARGET SDL2::SDL2-static)
        message(STATUS "Using system SDL2; skipping vendored ${SDL2_PATH}")
    else()
        if(EXISTS "${SDL2_PATH}/CMakeLists.txt")
            add_subdirectory(${SDL2_PATH})
        endif()
    endif()

    # SDL2_ttf: if system SDL2_ttf target exists, skip vendored (avoids building bundled freetype)
    if(TARGET SDL2_ttf)
        message(STATUS "Using system SDL2_ttf; skipping vendored ${SDL2_TTF_PATH}")
    else()
        if(EXISTS "${SDL2_TTF_PATH}/CMakeLists.txt")
            add_subdirectory(${SDL2_TTF_PATH})
        endif()
    endif()

    # SDL2_image
    if(TARGET SDL2_image)
        message(STATUS "Using system SDL2_image; skipping vendored ${SDL2_IMAGE_PATH}")
    else()
        if(EXISTS "${SDL2_IMAGE_PATH}/CMakeLists.txt")
            add_subdirectory(${SDL2_IMAGE_PATH})
        endif()
    endif()

    message(STATUS "External subdirectories setup complete")
endfunction()

message(STATUS "Dependencies configuration loaded successfully")
