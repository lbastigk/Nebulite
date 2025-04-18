cmake_minimum_required(VERSION 3.16)
project(Nebulite)

# Echo the start of the configuration
message(STATUS "Starting configuration for project Nebulite")

#  ▗▖ ▗▖▗▄▄▄▖▗▖  ▗▖▗▄▄▄  ▗▄▖ ▗▖ ▗▖ ▗▄▄▖     ▗▄▄▖▗▄▄▖ ▗▄▄▄▖ ▗▄▄▖▗▄▄▄▖▗▄▄▄▖▗▄▄▄▖ ▗▄▄▖
#  ▐▌ ▐▌  █  ▐▛▚▖▐▌▐▌  █▐▌ ▐▌▐▌ ▐▌▐▌       ▐▌   ▐▌ ▐▌▐▌   ▐▌     █  ▐▌     █  ▐▌
#  ▐▌ ▐▌  █  ▐▌ ▝▜▌▐▌  █▐▌ ▐▌▐▌ ▐▌ ▝▀▚▖     ▝▀▚▖▐▛▀▘ ▐▛▀▀▘▐▌     █  ▐▛▀▀▘  █  ▐▌
#  ▐▙█▟▌▗▄█▄▖▐▌  ▐▌▐▙▄▄▀▝▚▄▞▘▐▙█▟▌▗▄▄▞▘    ▗▄▄▞▘▐▌   ▐▙▄▄▖▝▚▄▄▖▗▄█▄▖▐▌   ▗▄█▄▖▝▚▄▄▖
#
#
#
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "If the build fails, please try using the POSIX threading model:")
    message(STATUS "sudo update-alternatives --config x86_64-w64-mingw32-g++")

    message(STATUS "Configuring for Windows")

    # Specify where to store the compiled binaries on Windows
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/)
    message(STATUS "Binary output directory set to: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

    # Include MinGW SDL2 directories explicitly
    include_directories(/usr/x86_64-w64-mingw32/include/SDL2)
    message(STATUS "Including MinGW SDL2 headers: /usr/x86_64-w64-mingw32/include/SDL2")

    # Link SDL2 libraries specific to MinGW
    set(SDL2_LIBS
        -L/usr/x86_64-w64-mingw32/lib
        -lmingw32
        -lSDL2main
        -lSDL2
        -lSDL2_ttf
        -lSDL2_image
    )
    message(STATUS "Linking MinGW SDL2 libraries from: /usr/x86_64-w64-mingw32/lib")

    # Set C++ standard for Windows
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    message(STATUS "C++20 standard set for Windows")

    # -----------------
    # For Threading
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)

    # Link with pthread and mthreads if necessary for MinGW
    add_compile_options(-pthread)  # Enable pthread support for MinGW
    add_compile_options(-mthreads)  # MinGW-specific thread support

    # Find the Threads package (ensures proper threading setup)
    find_package(Threads REQUIRED)

    # Ensure correct library linking for pthread and MinGW threading
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread -lstdc++")

    message(STATUS "Threading configured for MinGW (using -pthread and -mthreads)")

    # Ensure DLLs are found at runtime by adding the current directory to rpath
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,.")
    message(STATUS "Added runtime search path for DLLs: Current directory")

    # -----------------
    # Add custom commands to copy the necessary SDL2 DLLs to the output directory

    # Specify SDL2 DLL paths (modify these if necessary based on where your SDL2 DLLs are located)
    set(SDL2_DLL_PATH "/usr/x86_64-w64-mingw32/bin")

    # List of required DLLs
    set(SDL2_DLLS
        "${SDL2_DLL_PATH}/SDL2.dll"
        "${SDL2_DLL_PATH}/SDL2_ttf.dll"
        "${SDL2_DLL_PATH}/SDL2_image.dll"
    )

    message(STATUS "Copying necessary SDL2 DLLs to the runtime output directory")
#  ▗▖   ▗▄▄▄▖▗▖  ▗▖▗▖ ▗▖▗▖  ▗▖     ▗▄▄▖▗▄▄▖ ▗▄▄▄▖ ▗▄▄▖▗▄▄▄▖▗▄▄▄▖▗▄▄▄▖ ▗▄▄▖
#  ▐▌     █  ▐▛▚▖▐▌▐▌ ▐▌ ▝▚▞▘     ▐▌   ▐▌ ▐▌▐▌   ▐▌     █  ▐▌     █  ▐▌
#  ▐▌     █  ▐▌ ▝▜▌▐▌ ▐▌  ▐▌       ▝▀▚▖▐▛▀▘ ▐▛▀▀▘▐▌     █  ▐▛▀▀▘  █  ▐▌
#  ▐▙▄▄▖▗▄█▄▖▐▌  ▐▌▝▚▄▞▘▗▞▘▝▚▖    ▗▄▄▞▘▐▌   ▐▙▄▄▖▝▚▄▄▖▗▄█▄▖▐▌   ▗▄█▄▖▝▚▄▄▖
#
#
#
else()
    message(STATUS "Configuring for Linux")

    # Specify where to store the compiled binaries for Linux
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Application/bin)
    message(STATUS "Binary output directory set to: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

    # Link SDL2 libraries specific to Linux
    include_directories(/usr/include/SDL2)
    set(SDL2_LIBS SDL2 SDL2_ttf SDL2_image)
    message(STATUS "Linking SDL2 libraries: SDL2, SDL2_ttf, SDL2_image")

    # Set C++ standard for Linux
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    message(STATUS "C++20 standard set for Linux")

    add_compile_options(-g -w)  # Compiler options for Linux
    message(STATUS "Added compiler options: -g -w")
endif()

#  ▗▄▄▄▖▗▖  ▗▖ ▗▄▄▖▗▖   ▗▖ ▗▖▗▄▄▄ ▗▄▄▄▖ ▗▄▄▖
#    █  ▐▛▚▖▐▌▐▌   ▐▌   ▐▌ ▐▌▐▌  █▐▌   ▐▌
#    █  ▐▌ ▝▜▌▐▌   ▐▌   ▐▌ ▐▌▐▌  █▐▛▀▀▘ ▝▀▚▖
#  ▗▄█▄▖▐▌  ▐▌▝▚▄▄▖▐▙▄▄▖▝▚▄▞▘▐▙▄▄▀▐▙▄▄▖▗▄▄▞▘
#
#
#

# Include directories common for both platforms
message(STATUS "Setting Include directories")
include_directories(
    ${CMAKE_SOURCE_DIR}/Engine
    ${CMAKE_SOURCE_DIR}/Engine/Rendering
    ${CMAKE_SOURCE_DIR}/Engine/GameLoop
    ${CMAKE_SOURCE_DIR}/Engine/Helper
    ${CMAKE_SOURCE_DIR}/Engine/GameVars
    ${CMAKE_SOURCE_DIR}/Platform
    ${CMAKE_SOURCE_DIR}/Editor
    ${CMAKE_SOURCE_DIR}/Tests
    /usr/include/rapidjson
)

#   ▗▄▄▖ ▗▄▖ ▗▖ ▗▖▗▄▄▖  ▗▄▄▖▗▄▄▄▖    ▗▄▄▄▖▗▄▄▄▖▗▖   ▗▄▄▄▖ ▗▄▄▖
#  ▐▌   ▐▌ ▐▌▐▌ ▐▌▐▌ ▐▌▐▌   ▐▌       ▐▌     █  ▐▌   ▐▌   ▐▌
#   ▝▀▚▖▐▌ ▐▌▐▌ ▐▌▐▛▀▚▖▐▌   ▐▛▀▀▘    ▐▛▀▀▘  █  ▐▌   ▐▛▀▀▘ ▝▀▚▖
#  ▗▄▄▞▘▝▚▄▞▘▝▚▄▞▘▐▌ ▐▌▝▚▄▄▖▐▙▄▄▖    ▐▌   ▗▄█▄▖▐▙▄▄▖▐▙▄▄▖▗▄▄▞▘
#
#
#

# Common source files
message(STATUS "Setting source files in project")
set(COMMON_SOURCES
    ${CMAKE_SOURCE_DIR}/Platform/Platform.cpp
    ${CMAKE_SOURCE_DIR}/Editor/Editor.cpp
    ${CMAKE_SOURCE_DIR}/Editor/LevelEditor.cpp
    ${CMAKE_SOURCE_DIR}/Engine/GameVars/Basiswerte.cpp
    ${CMAKE_SOURCE_DIR}/Engine/GameVars/Eigenschaften.cpp
    ${CMAKE_SOURCE_DIR}/Engine/GameVars/InteractionRequirement.cpp
    ${CMAKE_SOURCE_DIR}/Engine/GameVars/Inventar.cpp
    ${CMAKE_SOURCE_DIR}/Engine/GameVars/Kreatur.cpp
    ${CMAKE_SOURCE_DIR}/Engine/GameVars/Quest.cpp
    ${CMAKE_SOURCE_DIR}/Engine/GameVars/Talente.cpp
    ${CMAKE_SOURCE_DIR}/Engine/GameVars/Zauber.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Helper/Calculator.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Helper/Console.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Helper/FileManagement.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Helper/FuncTree.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Helper/JSONHandler.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Helper/OptionsMenu.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Helper/StringHandler.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Helper/Time.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Rendering/Environment.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Rendering/MoveRuleSet.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Rendering/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Rendering/RenderObject.cpp
    ${CMAKE_SOURCE_DIR}/Tests/DsaDebug.cpp
    ${CMAKE_SOURCE_DIR}/Tests/Milestones.cpp
    ${CMAKE_SOURCE_DIR}/Tests/QuickNotes.cpp
    ${CMAKE_SOURCE_DIR}/Tests/TestEnv.cpp
    ${CMAKE_SOURCE_DIR}/Tests/_TEMPLATE_CLASS.cpp

)

#   ▗▄▖ ▗▖ ▗▖▗▄▄▄▖▗▄▄▖ ▗▖ ▗▖▗▄▄▄▖
#  ▐▌ ▐▌▐▌ ▐▌  █  ▐▌ ▐▌▐▌ ▐▌  █
#  ▐▌ ▐▌▐▌ ▐▌  █  ▐▛▀▘ ▐▌ ▐▌  █
#  ▝▚▄▞▘▝▚▄▞▘  █  ▐▌   ▝▚▄▞▘  █
#
#
#

message(STATUS "Setting targets...")

# Engine target
add_executable(Nebulite
    ${CMAKE_SOURCE_DIR}/Engine/main.cpp
    ${COMMON_SOURCES}
)
target_link_libraries(Nebulite ${SDL2_LIBS})

# Windows-Specific dll targets:
# Now, after the targets are created, add the custom commands
message(STATUS "Copying necessary SDL2 DLLs to the runtime output directory")
foreach(DLL ${SDL2_DLLS})
    add_custom_command(TARGET Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DLL}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
    )
    add_custom_command(TARGET Editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DLL}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
    )
    add_custom_command(TARGET TestEnvironment POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DLL}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
    )
endforeach()
