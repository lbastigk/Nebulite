if(NOT APPLE)
    message(WARNING "macbuild.cmake is intended for macOS builds only")
    return()
endif()

######################################################
message(STATUS "Loading macOS build configuration...")

# macOS-specific configuration
function(configure_macos target_name)
    message(STATUS "Configuring ${target_name} for macOS, build type: ${CMAKE_BUILD_TYPE}")

    # Frameworks needed on macOS
    target_link_libraries(${target_name} PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework Carbon"
        "-framework AudioToolbox"
        "-framework CoreAudio"
        "-framework ForceFeedback"
    )
    
    message(STATUS "${target_name} configured for macOS")
endfunction()

message(STATUS "macOS build configuration loaded successfully")