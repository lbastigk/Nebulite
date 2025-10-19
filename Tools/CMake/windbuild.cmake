# Windows-specific build configuration
# This file contains Windows-specific CMake configuration that can be included
# in the main CMakeLists.txt file

if(NOT WIN32)
    message(WARNING "windbuild.cmake is intended for Windows builds only")
    return()
endif()

message(STATUS "Loading Windows build configuration...")

# Windows-specific compile definitions
add_compile_definitions(WIN32_LEAN_AND_MEAN)
add_compile_definitions(NOMINMAX)

# Windows-specific SDL2 configuration using vcpkg or pre-built binaries
function(configure_windows_sdl2 target_name)
    message(STATUS "Configuring SDL2 for Windows target: ${target_name}")
    
    # Try to find SDL2 using pkg-config or CMake find modules first
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SDL2 QUIET sdl2)
        pkg_check_modules(SDL2_TTF QUIET SDL2_ttf)
        pkg_check_modules(SDL2_IMAGE QUIET SDL2_image)
    endif()
    
    if(SDL2_FOUND AND SDL2_TTF_FOUND AND SDL2_IMAGE_FOUND)
        message(STATUS "Found SDL2 via pkg-config")
        target_link_libraries(${target_name} PRIVATE
            ${SDL2_LIBRARIES}
            ${SDL2_TTF_LIBRARIES}
            ${SDL2_IMAGE_LIBRARIES}
            absl::flat_hash_map
        )
        target_include_directories(${target_name} PRIVATE
            ${SDL2_INCLUDE_DIRS}
            ${SDL2_TTF_INCLUDE_DIRS}
            ${SDL2_IMAGE_DIRS}
        )
    else()
        # Fallback: Try to use vcpkg or system-installed SDL2
        find_package(SDL2 QUIET)
        find_package(SDL2_ttf QUIET)
        find_package(SDL2_image QUIET)
        
        if(SDL2_FOUND AND SDL2_ttf_FOUND AND SDL2_image_FOUND)
            message(STATUS "Found SDL2 via CMake find modules")
            target_link_libraries(${target_name} PRIVATE
                SDL2::SDL2main
                SDL2::SDL2
                SDL2_ttf::SDL2_ttf
                SDL2_image::SDL2_image
                absl::flat_hash_map
            )
        else()
            message(FATAL_ERROR "SDL2 libraries not found for Windows build. Please install via vcpkg or provide pre-built libraries")
        endif()
    endif()

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
    )
    
    message(STATUS "SDL2 configured for Windows")
endfunction()

message(STATUS "Windows build configuration loaded successfully")