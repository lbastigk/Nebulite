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
    
    message(STATUS "Common dependencies configured for ${target_name}")
endfunction()

############################################################
# Function to setup external subdirectories (call once)
function(setup_external_subdirectories)
    message(STATUS "Setting up external subdirectories...")
    
    # Disable warnings for external dependencies
    set(CMAKE_CXX_FLAGS_BACKUP ${CMAKE_CXX_FLAGS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
    
    # Add Abseil subdirectory
    add_subdirectory(${ABSEIL_PATH})
    
    # SDL2 setup is handled in platform-specific configurations
    # to avoid duplicate target issues
    
    # Restore original flags
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_BACKUP})
    
    message(STATUS "External subdirectories setup complete")
endfunction()

message(STATUS "Dependencies configuration loaded successfully")