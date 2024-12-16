cmake_minimum_required(VERSION 3.5)

project(SDLEncodedQtApp)

# Find required packages
find_package(Qt5 REQUIRED COMPONENTS Widgets Gui)
find_package(SDL2 REQUIRED)

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
)

# Specify the header files for inclusion (optional but helps IDEs)
set(HEADERS
    ./_ExampleWindow.h
    ../QT/ImageWidget.h
    ../QT/ButtonWidget.h
    ../QT/SliderWidget.h
    ../QT/ExplorerWidget.h
)

# Create the executable
add_executable(app ${SOURCES})

# Include SDL2 and Qt5 directories
target_include_directories(app PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ./
    ../QT/
)

# Link the required libraries
target_link_libraries(app PRIVATE
    Qt5::Widgets
    Qt5::Gui
    ${SDL2_LIBRARIES}
)
