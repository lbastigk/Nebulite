cmake_minimum_required(VERSION 3.16)
project(Nebulite)

# Handle Windows-specific configurations
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Specify where to store the compiled binaries on Windows
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Application/bin)

    # Link SDL2 libraries specific to Windows (if necessary, link to MinGW version of SDL2)
    set(SDL2_LIBS SDL2 SDL2_ttf SDL2_image)

    # Set C++ standard for Windows
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
    # Specify where to store the compiled binaries for Linux
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Application/bin)

    # Link SDL2 libraries specific to Linux
    set(SDL2_LIBS SDL2 SDL2_ttf SDL2_image)

    # Set C++ standard for Linux
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    add_compile_options(-g -w)  # Compiler options for Linux
endif()

# Include directories common for both platforms
include_directories(
    ${CMAKE_SOURCE_DIR}/ProjectEngine
    ${CMAKE_SOURCE_DIR}/ProjectEngine/Rendering
    ${CMAKE_SOURCE_DIR}/ProjectEngine/GameLoop
    ${CMAKE_SOURCE_DIR}/ProjectEngine/Helper
    ${CMAKE_SOURCE_DIR}/ProjectEngine/GameVars
    ${CMAKE_SOURCE_DIR}/Platform
    ${CMAKE_SOURCE_DIR}/ProjectEditor
    ${CMAKE_SOURCE_DIR}/ProjectTestEnvironment
    /usr/include/SDL2
    /usr/include/rapidjson
)

# Common source files
set(COMMON_SOURCES
    ${CMAKE_SOURCE_DIR}/Platform/Platform.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEditor/Editor.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEditor/LevelEditor.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/GameLoop/GameLoop.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/GameLoop/OptionsLoader.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/GameLoop/Savegame.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/GameVars/Basiswerte.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/GameVars/Eigenschaften.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/GameVars/InteractionRequirement.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/GameVars/Inventar.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/GameVars/Kreatur.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/GameVars/Quest.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/GameVars/Talente.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/GameVars/Zauber.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/Helper/Calculator.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/Helper/Console.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/Helper/FileManagement.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/Helper/JSONHandler.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/Helper/OptionsMenu.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/Helper/StringHandler.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/Helper/Time.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/Rendering/Environment.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/Rendering/MoveRuleSet.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/Rendering/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/ProjectEngine/Rendering/RenderObject.cpp
    ${CMAKE_SOURCE_DIR}/ProjectTestEnvironment/DsaDebug.cpp
    ${CMAKE_SOURCE_DIR}/ProjectTestEnvironment/Milestones.cpp
    ${CMAKE_SOURCE_DIR}/ProjectTestEnvironment/QuickNotes.cpp
    ${CMAKE_SOURCE_DIR}/ProjectTestEnvironment/TestEnv.cpp
    ${CMAKE_SOURCE_DIR}/ProjectTestEnvironment/_TEMPLATE_CLASS.cpp
)

# Engine target
add_executable(Engine
    ${CMAKE_SOURCE_DIR}/ProjectEngine/main.cpp
    ${COMMON_SOURCES}
)
target_link_libraries(Engine ${SDL2_LIBS})

# Editor target
add_executable(Editor
    ${CMAKE_SOURCE_DIR}/ProjectEditor/main.cpp
    ${COMMON_SOURCES}
)
target_link_libraries(Editor ${SDL2_LIBS})

# Test Environment target
add_executable(TestEnvironment
    ${CMAKE_SOURCE_DIR}/ProjectTestEnvironment/main.cpp
    ${COMMON_SOURCES}
)
target_link_libraries(TestEnvironment ${SDL2_LIBS})

# Optional: Add a target to build all
add_custom_target(BuildAll
    DEPENDS Engine Editor TestEnvironment
)
