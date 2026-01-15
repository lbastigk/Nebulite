# Dependencies configuration
# This file handles external dependency paths and subdirectory inclusion

function(_detect_sdl3_targets)
    # local accumulators
    set(_found_core OFF)
    set(_found_ttf OFF)
    set(_found_image OFF)
    set(_link_libs)

    # Prefer well-known imported targets
    if(TARGET SDL3::SDL3)
        set(_found_core ON)
        list(APPEND _link_libs SDL3::SDL3)
    elseif(DEFINED SDL3_FOUND AND SDL3_FOUND)
        set(_found_core ON)
        if(DEFINED SDL3_LIBRARIES)
            list(APPEND _link_libs ${SDL3_LIBRARIES})
        endif()
    endif()

    if(TARGET SDL3_ttf::SDL3_ttf)
        set(_found_ttf ON)
        list(APPEND _link_libs SDL3_ttf::SDL3_ttf)
    elseif(DEFINED SDL3_TTF_FOUND AND SDL3_TTF_FOUND)
        set(_found_ttf ON)
        if(DEFINED SDL3_TTF_LIBRARIES)
            list(APPEND _link_libs ${SDL3_TTF_LIBRARIES})
        endif()
    endif()

    if(TARGET SDL3_image::SDL3_image)
        set(_found_image ON)
        list(APPEND _link_libs SDL3_image::SDL3_image)
    elseif(DEFINED SDL3_IMAGE_FOUND AND SDL3_IMAGE_FOUND)
        set(_found_image ON)
        if(DEFINED SDL3_IMAGE_LIBRARIES)
            list(APPEND _link_libs ${SDL3_IMAGE_LIBRARIES})
        endif()
    endif()

    # Export results to caller scope
    set(_SDL3_CORE_TARGET ${_found_core} PARENT_SCOPE)
    set(_SDL3_TTF_TARGET ${_found_ttf} PARENT_SCOPE)
    set(_SDL3_IMAGE_TARGET ${_found_image} PARENT_SCOPE)

    # Provide SDL3_LINK_LIBS for linking (empty if nothing found)
    if(_link_libs)
        # join list into a proper CMake list and export
        set(SDL3_LINK_LIBS ${_link_libs} PARENT_SCOPE)
    else()
        set(SDL3_LINK_LIBS "" PARENT_SCOPE)
    endif()

    message(STATUS "SDL3 detection: core=${_found_core} ttf=${_found_ttf} image=${_found_image}")
endfunction()

message(STATUS "Loading dependencies configuration...")

############################################################
# Shortcut paths to external dependencies
set(RAPIDJSON_PATH      "${CMAKE_SOURCE_DIR}/external/rapidjson")
set(TINYEXPR_PATH       "${CMAKE_SOURCE_DIR}/external/tinyexpr")
set(ABSEIL_PATH         "${CMAKE_SOURCE_DIR}/external/abseil")

# SDL3 official repositories
set(SDL3_PATH           "${CMAKE_SOURCE_DIR}/external/SDL3")
set(SDL3_TTF_PATH       "${CMAKE_SOURCE_DIR}/external/SDL3_ttf")
set(SDL3_IMAGE_PATH     "${CMAKE_SOURCE_DIR}/external/SDL3_image")

############################################################
# Required external dependencies for SDL3, prefer over system-installed
function(setup_bundled_externals_for BASE_PATH)
    set(EXTERNAL_DIR "${BASE_PATH}/external")

    if(NOT EXISTS "${EXTERNAL_DIR}")
        return()
    endif()

    # Collect children deterministically
    file(GLOB _children RELATIVE "${EXTERNAL_DIR}" "${EXTERNAL_DIR}/*")
    list(SORT _children)

    # Prefer freetype first (common ordering requirement)
    if(EXISTS "${EXTERNAL_DIR}/freetype/CMakeLists.txt")
        message(STATUS "Adding bundled external: ${EXTERNAL_DIR}/freetype")
        add_subdirectory("${EXTERNAL_DIR}/freetype" EXCLUDE_FROM_ALL)
    endif()

    foreach(_child IN LISTS _children)
        set(_subdir "${EXTERNAL_DIR}/${_child}")
        if(IS_DIRECTORY "${_subdir}" AND EXISTS "${_subdir}/CMakeLists.txt")
            if(_child STREQUAL "freetype")
                continue()
            endif()
            message(STATUS "Adding bundled external: ${_subdir}")
            add_subdirectory("${_subdir}" EXCLUDE_FROM_ALL)
        endif()
    endforeach()
endfunction()

# Replace the previous explicit adds with calls:
setup_bundled_externals_for(${SDL3_PATH})
setup_bundled_externals_for(${SDL3_TTF_PATH})
setup_bundled_externals_for(${SDL3_IMAGE_PATH})

############################################################
# SDL3 detection

find_package(SDL3 CONFIG QUIET)
_detect_sdl3_targets()

# If not found, try to add the bundled SDL3 subdirectory (if present)
if(NOT _SDL3_CORE_TARGET)
    if(EXISTS "${SDL3_PATH}/CMakeLists.txt")
        message(STATUS "SDL3 not found via find_package; adding bundled SDL3 from ${SDL3_PATH}")
        add_subdirectory(${SDL3_PATH})
        _detect_sdl3_targets()
    endif()
endif()

# Also try bundled extensions (ttf/image) if core still not found or if they provide targets
if(NOT _SDL3_TTF_TARGET AND EXISTS "${SDL3_TTF_PATH}/CMakeLists.txt")
    add_subdirectory(${SDL3_TTF_PATH})
    _detect_sdl3_targets()
endif()
if(NOT _SDL3_IMAGE_TARGET AND EXISTS "${SDL3_IMAGE_PATH}/CMakeLists.txt")
    add_subdirectory(${SDL3_IMAGE_PATH})
    _detect_sdl3_targets()
endif()

if(NOT _SDL3_CORE_TARGET)
    message(FATAL_ERROR "SDL3 core target not found. Inspect external/SDL3 CMakeLists or install SDL3 or use find_package(SDL3 CONFIG REQUIRED).")
endif()

############################################################
# Abseil libraries
set(ABSL_LINK_LIBS
        absl::flat_hash_map
)

############################################################
# Function to configure common dependencies for a target
function(configure_common_dependencies target_name)
    message(STATUS "Configuring common dependencies for target: ${target_name}")

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
            ${SDL3_PATH}/include
            ${SDL3_TTF_PATH}/include
            ${SDL3_IMAGE_PATH}/include
    )

    # Show SDL3 paths and halt for debugging
    message(STATUS "SDL3 include path: ${SDL3_PATH}/include")
    message(STATUS "SDL3_ttf include path: ${SDL3_TTF_PATH}/include")
    message(STATUS "SDL3_image include path: ${SDL3_IMAGE_PATH}/include")
    # Debug halt (uncomment if needed)
    #message(FATAL_ERROR "Debug halt after SDL3 include paths")

    # Link libraries
    target_link_libraries(${target_name} PRIVATE
            ${SDL3_LINK_LIBS}
            ${ABSL_LINK_LIBS}
    )

    message(STATUS "Common dependencies configured for ${target_name}")
endfunction()

############################################################
# Function to setup external subdirectories (call once)
function(setup_external_subdirectories)
    message(STATUS "Setting up external subdirectories...")

    # Add Abseil subdirectory if present and target not already available
    if(NOT TARGET absl::flat_hash_map AND EXISTS "${ABSEIL_PATH}/CMakeLists.txt")
        add_subdirectory(${ABSEIL_PATH})
    endif()

    # Include shared SDL3/SDL3_image/SDL3_ttf build settings
    sdl_setup()

    # Add SDL3 subdirectories only if targets are not already defined
    if(NOT TARGET SDL3::SDL3 AND EXISTS "${SDL3_PATH}/CMakeLists.txt")
        add_subdirectory(${SDL3_PATH})
    endif()
    if(NOT TARGET SDL3_ttf::SDL3_ttf AND EXISTS "${SDL3_TTF_PATH}/CMakeLists.txt")
        add_subdirectory(${SDL3_TTF_PATH})
    endif()
    if(NOT TARGET SDL3_image::SDL3_image AND EXISTS "${SDL3_IMAGE_PATH}/CMakeLists.txt")
        add_subdirectory(${SDL3_IMAGE_PATH})
    endif()

    message(STATUS "External subdirectories setup complete")
endfunction()

message(STATUS "Dependencies configuration loaded successfully")