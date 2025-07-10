############################################################
# General
cmake_minimum_required(VERSION 3.16)
project(Nebulite)

add_definitions(-DSDL_DISABLE_IMPLEMENTS)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-g -w)

# Echo the start of the configuration
message(STATUS "Starting configuration for project Nebulite")

# Detect platform
if(WIN32)
    set(IS_WINDOWS TRUE)
else()
    set(IS_WINDOWS FALSE)
endif()

# Set binary output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Application/bin)
message(STATUS "Binary output directory set to: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

############################################################
# Paths to external libs
set(SDL2_BUILD_PATH "${CMAKE_SOURCE_DIR}/external/SDL2_build")
set(RAPIDJSON_PATH  "${CMAKE_SOURCE_DIR}/external/rapidjson")
set(EXPRTK_PATH     "${CMAKE_SOURCE_DIR}/external/exprtk")
set(TINYEXPR_PATH   "${CMAKE_SOURCE_DIR}/external/tinyexpr")
set(ABSEIL_PATH     "${CMAKE_SOURCE_DIR}/external/abseil")

# Add abseil subdirectory
add_subdirectory(${ABSEIL_PATH} ${CMAKE_BINARY_DIR}/external/abseil_build)

############################################################
# General include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src/Engine
    ${CMAKE_SOURCE_DIR}/src/Engine/Data
    ${CMAKE_SOURCE_DIR}/src/Engine/Rendering
    ${CMAKE_SOURCE_DIR}/src/Engine/Helper
    ${CMAKE_SOURCE_DIR}/src/Platform
    ${CMAKE_SOURCE_DIR}/src/Editor
    ${CMAKE_SOURCE_DIR}/src/Tests
    ${RAPIDJSON_PATH}/include/rapidjson
    ${EXPRTK_PATH}
    ${TINYEXPR_PATH}
    ${ABSEIL_PATH}
)

############################################################
# Source files
set(COMMON_SOURCES
    ${TINYEXPR_PATH}/tinyexpr.c
    ${CMAKE_SOURCE_DIR}/src/Platform/Platform.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Nebulite.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Data/JSON.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Helper/FileManagement.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Helper/FuncTree.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Helper/StringHandler.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Helper/Time.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Rendering/Environment.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Rendering/Invoke.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Rendering/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Rendering/RenderObject.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Rendering/RenderObjectContainer.cpp
)

############################################################
# Define executable
add_executable(Nebulite
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${COMMON_SOURCES}
)


############################################################
# Link libraries
target_link_libraries(Nebulite PRIVATE
    absl::flat_hash_map
    absl::base
    absl::strings
)

############################################################
# Platform-Specific
if(IS_WINDOWS)
    message(STATUS "Targeting Windows: linking SDL static libraries")
    set(CMAKE_SYSTEM_NAME Windows)

    target_include_directories(Nebulite PRIVATE
        ${SDL2_BUILD_PATH}/shared_windows/include/SDL2
    )

    target_link_libraries(Nebulite PRIVATE
        -lmingw32
        "${SDL2_BUILD_PATH}/shared_windows/lib/libSDL2main.a"
        "${SDL2_BUILD_PATH}/shared_windows/lib/libSDL2_ttf.dll.a"
        "${SDL2_BUILD_PATH}/shared_windows/lib/libSDL2_image.dll.a"
        "${SDL2_BUILD_PATH}/shared_windows/lib/libSDL2.dll.a"
        -mwindows
        -lsetupapi
        -limm32
        -lversion
        -loleaut32
        -lole32
        -luuid
        -lwinmm
        -lws2_32
    )

    ###################################
    # Copy SDL dlls
    add_custom_command(TARGET Nebulite POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDL2_BUILD_PATH}/shared_windows/bin/SDL2.dll"
            "${SDL2_BUILD_PATH}/shared_windows/bin/SDL2_ttf.dll"
            "${SDL2_BUILD_PATH}/shared_windows/bin/SDL2_image.dll"
            "${CMAKE_SOURCE_DIR}/Application/bin"
        COMMENT "Copying SDL DLLs to output directory"
    )

    ###################################
    # Copy necessary windows dlls

    # [TODO] No libwinpthread-1.dll exists under /usr/*mingw*/bin
    # the only files existing are:
    #
    # $ find /usr -name "libwinpthread-1.dll" 2>/dev/null
    #   > /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll
    #   > /usr/i686-w64-mingw32/lib/libwinpthread-1.dll

    # Try to find libwinpthread-1.dll in common locations
    # Assuming MinGW toolchain environment variable or standard install path
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(MINGW_LIB_PATH "/usr/x86_64-w64-mingw32/bin")
    else()
        set(MINGW_LIB_PATH "/usr/i686-w64-mingw32/bin")
    endif()

    set(WINPTHREAD_DLL "${MINGW_LIB_PATH}/libwinpthread-1.dll")

    if(EXISTS ${WINPTHREAD_DLL})
        add_custom_command(TARGET Nebulite POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${WINPTHREAD_DLL}"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libwinpthread-1.dll"
        )
        message(STATUS "libwinpthread-1.dll will be copied from: ${WINPTHREAD_DLL}")
    else()
        message(WARNING "libwinpthread-1.dll not found in expected path: ${WINPTHREAD_DLL}")
    endif()
else()
    message(STATUS "Targeting Linux: linking SDL static libraries")

    target_include_directories(Nebulite PRIVATE
        ${SDL2_BUILD_PATH}/shared/include/SDL2
    )

    target_link_libraries(Nebulite PRIVATE
        "${SDL2_BUILD_PATH}/static/lib/libSDL2main.a"
        "${SDL2_BUILD_PATH}/static/lib/libSDL2_ttf.a"
        "${SDL2_BUILD_PATH}/static/lib/libSDL2_image.a"
        "${SDL2_BUILD_PATH}/static/lib/libSDL2.a"
    )
endif()

