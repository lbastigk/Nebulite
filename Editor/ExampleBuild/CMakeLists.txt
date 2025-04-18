cmake_minimum_required(VERSION 3.5)

project(SDLEncodedQtApp)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(Qt5 REQUIRED COMPONENTS Widgets Gui)
find_package(SDL2 REQUIRED)

# Include directories
include_directories(/usr/include/SDL2)

# SDL2 libraries
set(SDL2_LIBS SDL2 SDL2_ttf SDL2_image)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)

# Source files
set(SOURCES
    ./main.cpp
    ./EditorApp.cpp
    ./EditorTab_LevelEditor.cpp
    ./EditorTab_RenderObjectEditor.cpp
    ./EditorTab_MoveRuleSetEditor.cpp

    ../QT/ImageWidget.cpp
    ../QT/ButtonWidget.cpp
    ../QT/SliderWidget.cpp
    ../QT/ExplorerWidget.cpp

    ../../Platform/Platform.cpp

    ../../Engine/Helper/Calculator.cpp
    ../../Engine/Helper/FileManagement.cpp
    ../../Engine/Helper/JSONHandler.cpp
    ../../Engine/Helper/StringHandler.cpp
    ../../Engine/Helper/Time.cpp

    ../../Engine/Rendering/Environment.cpp
    ../../Engine/Rendering/Renderer.cpp
    ../../Engine/Rendering/RenderObject.cpp
    ../../Engine/Rendering/MoveRuleSet.cpp
)

# Create executable
add_executable(app ${SOURCES})

# Include directories
target_include_directories(app PRIVATE
    ${SDL2_INCLUDE_DIRS}
    /usr/include/rapidjson/
    ./
    ../QT/
    ../../Platform/
    ../../Engine/GameLoop/
    ../../Engine/Helper/
    ../../Engine/Rendering/
)

# Link libraries
target_link_libraries(app PRIVATE
    Qt5::Widgets
    Qt5::Gui
    ${SDL2_LIBRARIES}
    ${SDL2_LIBS}
)

# Optional: Compiler options
if(NOT WIN32)
    add_compile_options(-g -w)
endif()
