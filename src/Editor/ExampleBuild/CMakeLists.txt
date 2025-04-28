cmake_minimum_required(VERSION 3.5)

project(app)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)

# using the generated QT5 Files...
set(BASE_DIR "${CMAKE_SOURCE_DIR}/../..")
set(SDL2_PATH       "${BASE_DIR}/external/SDL2_build")
set(RAPIDJSON_PATH  "${BASE_DIR}/external/rapidjson")

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui)



# Ensure CMake uses the correct Qt5 installation path
set(CMAKE_PREFIX_PATH "${QT5_PATH}")

# Find the necessary Qt5 components
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui)

#####################
# [INCLUDES]

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../QT
    ${BASE_DIR}/Engine
    ${BASE_DIR}/Engine/Rendering
    ${BASE_DIR}/Engine/GameLoop
    ${BASE_DIR}/Engine/Helper
    ${BASE_DIR}/Engine/GameVars
    ${BASE_DIR}/Platform
    ${BASE_DIR}/Editor
    ${BASE_DIR}/Tests
    ${SDL2_PATH}/include/SDL2/
    ${RAPIDJSON_PATH}/include/rapidjson
    ${QT5_PATH}/include
    ${QT5_PATH}/include/QtCore
    ${QT5_PATH}/include/QtGui
    ${QT5_PATH}/include/QtWidgets
)

#####################
# [SOURCES]

# Common source files
message(STATUS "Setting source files in project")
set(COMMON_SOURCES
    ${BASE_DIR}/Platform/Platform.cpp
    ${BASE_DIR}/Editor/Editor.cpp
    ${BASE_DIR}/Editor/LevelEditor.cpp
    ${BASE_DIR}/Engine/GameVars/Basiswerte.cpp
    ${BASE_DIR}/Engine/GameVars/Eigenschaften.cpp
    ${BASE_DIR}/Engine/GameVars/InteractionRequirement.cpp
    ${BASE_DIR}/Engine/GameVars/Inventar.cpp
    ${BASE_DIR}/Engine/GameVars/Kreatur.cpp
    ${BASE_DIR}/Engine/GameVars/Quest.cpp
    ${BASE_DIR}/Engine/GameVars/Talente.cpp
    ${BASE_DIR}/Engine/GameVars/Zauber.cpp
    ${BASE_DIR}/Engine/Helper/Calculator.cpp
    ${BASE_DIR}/Engine/Helper/Console.cpp
    ${BASE_DIR}/Engine/Helper/FileManagement.cpp
    ${BASE_DIR}/Engine/Helper/FuncTree.cpp
    ${BASE_DIR}/Engine/Helper/JSONHandler.cpp
    ${BASE_DIR}/Engine/Helper/OptionsMenu.cpp
    ${BASE_DIR}/Engine/Helper/StringHandler.cpp
    ${BASE_DIR}/Engine/Helper/Time.cpp
    ${BASE_DIR}/Engine/Rendering/Environment.cpp
    ${BASE_DIR}/Engine/Rendering/MoveRuleSet.cpp
    ${BASE_DIR}/Engine/Rendering/Renderer.cpp
    ${BASE_DIR}/Engine/Rendering/RenderObject.cpp
    ${BASE_DIR}/Tests/DsaDebug.cpp
    ${BASE_DIR}/Tests/Milestones.cpp
    ${BASE_DIR}/Tests/QuickNotes.cpp
    ${BASE_DIR}/Tests/TestEnv.cpp
    ${BASE_DIR}/Tests/_TEMPLATE_CLASS.cpp
    ./main.cpp
    ./EditorApp.cpp
    ./EditorTab_LevelEditor.cpp
    ./EditorTab_RenderObjectEditor.cpp
    ./EditorTab_MoveRuleSetEditor.cpp
    ../QT/ImageWidget.cpp
    ../QT/ButtonWidget.cpp
    ../QT/SliderWidget.cpp
    ../QT/ExplorerWidget.cpp
)

#####################
# [OUTPUT]

message(STATUS "Setting targets...")

# Create executable
add_executable(app
    ${CMAKE_SOURCE_DIR}/main.cpp
    ${COMMON_SOURCES}
)

# Link the Qt5 libraries
target_link_libraries(app PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)

# Link the SDL2 libraries (make sure these paths are correct for your system)
target_link_libraries(app PRIVATE
    ${SDL2_PATH}/lib/libSDL2.a
    ${SDL2_PATH}/lib/libSDL2_ttf.a
    ${SDL2_PATH}/lib/libSDL2_image.a
)

# Optional: Compiler options for non-Windows systems
if(NOT WIN32)
    add_compile_options(-g -w)
endif()
