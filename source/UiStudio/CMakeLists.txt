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
    MainWindow.h MainWindow.cpp
)

target_link_libraries(UiStudio PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
