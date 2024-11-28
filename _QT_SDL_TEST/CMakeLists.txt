cmake_minimum_required(VERSION 3.5)

project(SDLEncodedQtApp)

find_package(Qt5 REQUIRED COMPONENTS Widgets Gui)
find_package(SDL2 REQUIRED)

set(CMAKE_AUTOMOC ON)

add_executable(SDLEncodedQtApp src/main.cpp)

target_include_directories(SDLEncodedQtApp PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(SDLEncodedQtApp Qt5::Widgets Qt5::Gui ${SDL2_LIBRARIES})
