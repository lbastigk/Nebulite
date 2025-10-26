# Dependencies configuration
# This file handles external dependency paths and subdirectory inclusion

message(STATUS "Loading dependencies configuration...")

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
# Linked libraries from all external dependencies

# SDL2 related libraries
set(SDL2_LINK_LIBS
    SDL2::SDL2main
    SDL2::SDL2-static
    SDL2_ttf
    SDL2_image
)

# Abseil libraries
set(ABSL_LINK_LIBS
    absl::flat_hash_map
)

############################################################
# Function to configure common dependencies for a target
function(configure_common_dependencies target_name)
    message(STATUS "Configuring common dependencies for target: ${target_name}")
    
    # Include directories
    target_include_directories(${target_name}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
        SYSTEM
            ${RAPIDJSON_PATH}/include
            ${EXPRTK_PATH}
            ${TINYEXPR_PATH}
            ${ABSEIL_PATH}
    )

    # Link common libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${SDL2_LINK_LIBS}
        ${ABSL_LINK_LIBS}
    )
    
    message(STATUS "Common dependencies configured for ${target_name}")
endfunction()

############################################################
# Function to setup external subdirectories (call once)
function(setup_external_subdirectories)
    message(STATUS "Setting up external subdirectories...")
    
    # Add Abseil subdirectory
    add_subdirectory(${ABSEIL_PATH})

    # Include shared SDL2/SDL2_image/SDL2_ttf build settings
    sdl_setup()

    # Add SDL2 subdirectories in correct order (SDL2 first, then extensions)
    add_subdirectory(${SDL2_PATH})
    add_subdirectory(${SDL2_TTF_PATH})
    add_subdirectory(${SDL2_IMAGE_PATH})

    message(STATUS "External subdirectories setup complete")
endfunction()

message(STATUS "Dependencies configuration loaded successfully")