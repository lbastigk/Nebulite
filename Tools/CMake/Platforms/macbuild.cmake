# macOS-specific build configuration
# This file contains macOS-specific CMake configuration

if(NOT APPLE)
    message(WARNING "macbuild.cmake is intended for macOS builds only")
    return()
endif()

message(STATUS "Loading macOS build configuration...")

# macOS-specific compile definitions (if any)
# add_compile_definitions(MACOS_SPECIFIC_FLAG)

# macOS-specific SDL3 configuration
function(configure_macos target_name)
    message(STATUS "Configuring ${target_name} for macOS, build type: ${CMAKE_BUILD_TYPE}")
    
    # Use the same SDL3 submodules as Linux/Windows
    message(STATUS "Using SDL3 submodules for macOS cross-compilation")

    # Frameworks needed for SDL3 on macOS
    target_link_libraries(${target_name} PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework Carbon"
        "-framework AudioToolbox"
        "-framework CoreAudio"
        "-framework ForceFeedback"
    )
    
    message(STATUS "SDL3 configured for macOS")
endfunction()

message(STATUS "macOS build configuration loaded successfully")