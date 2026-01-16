if(NOT UNIX OR APPLE)
    message(WARNING "linuxbuild.cmake is intended for Linux builds only")
    return()
endif()

######################################################
message(STATUS "Loading Linux build configuration...")

# Linux-specific configuration
function(configure_linux target_name)
    message(STATUS "Configuring ${target_name} for Linux, build type: ${CMAKE_BUILD_TYPE}")
    
    # Additional system libraries needed for static linking on Linux
    target_link_libraries(${target_name} PRIVATE
        m           # Math library
        dl          # Dynamic loading
        pthread     # Threading
    )
    
    message(STATUS "${target_name} configured for Linux")
endfunction()

message(STATUS "Linux build configuration loaded successfully")