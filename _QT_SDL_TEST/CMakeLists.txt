cmake_minimum_required(VERSION 3.5)

project(SDLEncodedQtApp)

# Find required packages
find_package(Qt5 REQUIRED COMPONENTS Widgets Gui)
find_package(SDL2 REQUIRED)

# Enable automatic Qt meta-object code generation
set(CMAKE_AUTOMOC ON)

# Specify the source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/ImageWidget.cpp
    src/ButtonWidget.cpp
    src/SliderWidget.cpp
    src/ExplorerWidget.cpp
)

# Specify the header files for inclusion (optional but helps IDEs)
set(HEADERS
    src/MainWindow.h
    src/ImageWidget.h
    src/ButtonWidget.h
    src/SliderWidget.h
    src/ExplorerWidget.h
)

# Create the executable
add_executable(app ${SOURCES})

# Include SDL2 and Qt5 directories
target_include_directories(app PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link the required libraries
target_link_libraries(app PRIVATE
    Qt5::Widgets
    Qt5::Gui
    ${SDL2_LIBRARIES}
)
