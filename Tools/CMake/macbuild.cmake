# macOS-specific build configuration
# This file contains macOS-specific CMake configuration

if(NOT APPLE)
    message(WARNING "macbuild.cmake is intended for macOS builds only")
    return()
endif()

message(STATUS "Loading macOS build configuration...")

# macOS-specific compile definitions (if any)
# add_compile_definitions(MACOS_SPECIFIC_FLAG)

# macOS-specific SDL2 configuration
function(configure_macos_sdl2 target_name)
    message(STATUS "Configuring SDL2 for macOS target: ${target_name}")
    
    # Add SDL2 subdirectories and link directly
    add_subdirectory(${SDL2_PATH} SDL2 EXCLUDE_FROM_ALL)
    add_subdirectory(${SDL2_TTF_PATH} SDL2_ttf EXCLUDE_FROM_ALL)
    add_subdirectory(${SDL2_IMAGE_PATH} SDL2_image EXCLUDE_FROM_ALL)

    # Link SDL2 libraries
    target_link_libraries(${target_name} PRIVATE 
        SDL2::SDL2main
        SDL2::SDL2
        SDL2_ttf::SDL2_ttf
        SDL2_image::SDL2_image
        absl::flat_hash_map
    )

    # Frameworks needed for SDL2 on macOS
    target_link_libraries(${target_name} PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework Carbon"
        "-framework AudioToolbox"
        "-framework CoreAudio"
        "-framework ForceFeedback"
    )
    
    message(STATUS "SDL2 configured for macOS")
endfunction()

message(STATUS "macOS build configuration loaded successfully")