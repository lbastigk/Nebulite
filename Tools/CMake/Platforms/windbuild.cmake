if(NOT WIN32)
    message(WARNING "windbuild.cmake is intended for Windows builds only")
    return()
endif()

######################################################
message(STATUS "Loading Windows build configuration...")

# Windows-specific configuration
function(configure_windows target_name)
    message(STATUS "Configuring ${target_name} for Windows, build type: ${CMAKE_BUILD_TYPE}")

    # Windows-specific compile definitions
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
    add_compile_definitions(NOMINMAX)
    
    # Windows system libraries needed for static linking
    target_link_libraries(${target_name} PRIVATE
        ws2_32      # Winsock
        winmm       # Windows multimedia
        setupapi    # Setup API
        version     # Version info
    )

    # Static linking flags to avoid DLL dependencies
    target_link_options(${target_name} PRIVATE
        -static-libgcc
        -static-libstdc++
        -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive
    )
    message(STATUS "Windows build: Static linking enabled (no DLL dependencies)")

    # Force console window in Debug, GUI (no console) in Release
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_options(${target_name} PRIVATE -mconsole)
        message(STATUS "Windows Debug build: Console enabled")
    else()
        # For release builds - currently keeping console for output
        target_link_options(${target_name} PRIVATE -mconsole)
        message(STATUS "Windows Release build: Console enabled (for debugging)")
        
        # Uncomment for true GUI release (no console):
        # target_link_options(${target_name} PRIVATE -mwindows)
    endif()

    # Link Windows-specific system libraries
    target_link_libraries(${target_name} PRIVATE
        -lmingw32
        -lsetupapi
        -limm32
        -lversion
        -loleaut32
        -lole32
        -luuid
        -lwinmm
        -lws2_32
        -lshlwapi  # Required for PathMatchSpecA (used by include/Utility/Glob.hpp)
    )
endfunction()

message(STATUS "Windows build configuration loaded successfully")