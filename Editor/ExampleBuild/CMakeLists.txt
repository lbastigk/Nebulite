cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 20)        # Specifies C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # Enforces the specified standard
set(CMAKE_CXX_EXTENSIONS OFF)        # Disables compiler-specific extensions (optional, for portability)


project(SDLEncodedQtApp)

# Find required packages
find_package(Qt5 REQUIRED COMPONENTS Widgets Gui)
find_package(SDL2 REQUIRED)

# Include directories for SDL_image and SDL_ttf
include_directories(/usr/include/SDL2)

# Link SDL2 libraries, including SDL_image and SDL_ttf
set(SDL2_LIBS SDL2 SDL2_ttf SDL2_image)

# Enable automatic Qt meta-object code generation
set(CMAKE_AUTOMOC ON)

# Specify the source files
set(SOURCES
    ./_ExampleMain.cpp
    ./_ExampleWindow.cpp
    ../QT/ImageWidget.cpp
    ../QT/ButtonWidget.cpp
    ../QT/SliderWidget.cpp
    ../QT/ExplorerWidget.cpp
    ../../Platform/Platform.cpp
    ../../ProjectEngine/GameLoop/OptionsLoader.cpp
    ../../ProjectEngine/Helper/Calculator.cpp
    ../../ProjectEngine/Helper/FileManagement.cpp
    ../../ProjectEngine/Helper/JSONHandler.cpp
    ../../ProjectEngine/Helper/StringHandler.cpp
    ../../ProjectEngine/Helper/Time.cpp
    ../../ProjectEngine/Rendering/Environment.cpp
    ../../ProjectEngine/Rendering/Renderer.cpp
    ../../ProjectEngine/Rendering/RenderObject.cpp
    ../../ProjectEngine/Rendering/MoveRuleSet.cpp
)


# Create the executable
add_executable(app ${SOURCES})

# Include SDL2 and Qt5 directories
target_include_directories(app PRIVATE
    ${SDL2_INCLUDE_DIRS}
    /usr/include/rapidjson/
    ./
    ../QT/
    ../../Platform/
    ../../ProjectEngine/GameLoop/
    ../../ProjectEngine/Helper/
    ../../ProjectEngine/Rendering/
)

# Link the required libraries
target_link_libraries(app PRIVATE
    Qt5::Widgets
    Qt5::Gui
    ${SDL2_LIBRARIES}
    ${SDL2_LIBS}
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler options for Linux
if(NOT WIN32)
    add_compile_options(-g -w)
endif()
