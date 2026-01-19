############################################################
# Shortcut paths to external dependencies

# SDL3 official repositories
set(SDL3_PATH           "${CMAKE_SOURCE_DIR}/external/SDL3")
set(SDL3_TTF_PATH       "${CMAKE_SOURCE_DIR}/external/SDL3_ttf")
set(SDL3_IMAGE_PATH     "${CMAKE_SOURCE_DIR}/external/SDL3_image")

# GUI libraries
set(IMGUI_PATH          "${CMAKE_SOURCE_DIR}/external/imgui")

# Other external libraries
set(RAPIDJSON_PATH      "${CMAKE_SOURCE_DIR}/external/rapidjson")
set(TINYEXPR_PATH       "${CMAKE_SOURCE_DIR}/external/tinyexpr")
set(ABSEIL_PATH         "${CMAKE_SOURCE_DIR}/external/abseil")

############################################################
# Function to setup external subdirectories
function(setup_external_subdirectories)
    message(STATUS "Setting up external subdirectories...")
    add_subdirectory(${ABSEIL_PATH})
    add_subdirectory(${SDL3_PATH})
    add_subdirectory(${SDL3_TTF_PATH})
    add_subdirectory(${SDL3_IMAGE_PATH})
    message(STATUS "External subdirectories setup complete")
endfunction()

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
            ${SDL3_PATH}/include/
            ${SDL3_TTF_PATH}/include
            ${SDL3_IMAGE_PATH}/include
            ${IMGUI_PATH}
    )

    # Link libraries
    target_link_libraries(${target_name} PRIVATE
            SDL3::SDL3
            SDL3_ttf::SDL3_ttf
            SDL3_image::SDL3_image
            absl::flat_hash_map
    )

    message(STATUS "Common dependencies configured for ${target_name}")
endfunction()