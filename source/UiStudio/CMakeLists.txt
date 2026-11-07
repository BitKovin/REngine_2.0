cmake_minimum_required(VERSION 3.19)
project(UiStudio LANGUAGES CXX)

# UI Studio is a standalone Qt tool. It links no engine or game code: it talks to a
# game process over its console (-console_stdin) and reads the files the game writes.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets)
qt_standard_project_setup()

qt_add_executable(UiStudio
    WIN32
    main.cpp
    Theme.h Theme.cpp
    Highlighter.h Highlighter.cpp
    CodeEditor.h CodeEditor.cpp
    GameSession.h GameSession.cpp
    PreviewView.h PreviewView.cpp
    Templates.h Templates.cpp
    TestPanel.h TestPanel.cpp
    XmlModel.h XmlModel.cpp
    UiDatabase.h UiDatabase.cpp
    PageTree.h PageTree.cpp
    PropertiesPanel.h PropertiesPanel.cpp
    EmbeddedGameView.h EmbeddedGameView.cpp
    VisualEdit.cpp
    MainWindow.h MainWindow.cpp
)

target_link_libraries(UiStudio PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

# Copy the Qt runtime next to the program after every build, for the configuration being built.
if(WIN32 AND TARGET Qt6::windeployqt)
    add_custom_command(TARGET UiStudio POST_BUILD
        COMMAND Qt6::windeployqt --no-translations --no-system-d3d-compiler --no-opengl-sw
                $<IF:$<CONFIG:Debug>,--debug,--release> $<TARGET_FILE:UiStudio>
        COMMENT "Deploying the Qt runtime")
endif()
