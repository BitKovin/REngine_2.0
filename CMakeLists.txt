cmake_minimum_required(VERSION 3.16)
project(MyProject)

# Set C++ standard and global flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build")

option(JS_ONLY "Compiles to native JS (No WASM)" OFF)

# Global compile definitions and flags.
add_definitions(-DNDEBUG)
add_definitions(-O3)
add_definitions(-std=c++20)

# Global include directories.
include_directories("${CMAKE_SOURCE_DIR}/librariesEms")

###############################################################################
# Engine Setup: Build the engine as a static library
###############################################################################
# Gather engine source files (excluding directories like glm as needed)
file(GLOB_RECURSE ENGINE_SOURCES "${CMAKE_SOURCE_DIR}/source/Engine/*.cpp")
set(ENGINE_SRC "")
foreach(source_file IN LISTS ENGINE_SOURCES)
    if(NOT source_file MATCHES "/glm/")
        list(APPEND ENGINE_SRC ${source_file})
    endif()
endforeach()

# Create the static library target for the engine
add_library(engine STATIC ${ENGINE_SRC})

# Set the engine's public include directory.
# Use the directory where the engine headers reside. Adjust if your headers are in a subfolder.
target_include_directories(engine PUBLIC "${CMAKE_SOURCE_DIR}/source/Engine")

# Link third-party libraries to the engine
target_link_libraries(engine PRIVATE
    "${CMAKE_SOURCE_DIR}/libsEms/libassimp.a"
    "${CMAKE_SOURCE_DIR}/libsEms/libIrrXML.a"
    "${CMAKE_SOURCE_DIR}/libsEms/libJolt.a"
)

# Apply WASM-specific target properties and linker options if not building JS only
if (NOT JS_ONLY)
    message(STATUS "Setting compilation target to WASM for Engine")
    set_target_properties(engine PROPERTIES OUTPUT_NAME "engine")
    target_link_options(engine PRIVATE
        "SHELL:-s USE_SDL=2"
        "SHELL:-s USE_SDL_IMAGE=2"
        "SHELL:-s USE_SDL_TTF=2"
        "SHELL:-s USE_SDL_MIXER=2"
	"SHELL:-s SDL2_IMAGE_FORMATS=[\"png\",\"jpg\",\"tga\"]"
        "SHELL:-s USE_WEBGL2=1"
        "SHELL:-s MIN_WEBGL_VERSION=2"
        "SHELL:-s MAX_WEBGL_VERSION=2"
        "SHELL:-s GL_PREINITIALIZED_CONTEXT=1"
        "SHELL:-s STACK_SIZE=500000"
        "SHELL:-s INITIAL_HEAP=83886080"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:-s NO_DISABLE_EXCEPTION_CATCHING"
        "SHELL:-s WASM=1"
        "SHELL:-s ASSERTIONS=1"
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/source/GameData@/GameData"
        "SHELL:-s GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=1"
        "SHELL:-lidbfs.js"
    )
endif()

###############################################################################
# Game Setup: Build the game as an executable linking against the engine
###############################################################################
# Gather game source files (adjust the path as necessary)
file(GLOB_RECURSE GAME_SOURCES "${CMAKE_SOURCE_DIR}/source/Game/*.cpp")

# Create the executable target for the game
add_executable(game ${GAME_SOURCES})

# The game target already inherits the PUBLIC include directories of engine
# (i.e. "${CMAKE_SOURCE_DIR}/source/Engine") when linking to engine.
# You can add additional include directories for game if necessary:
target_include_directories(game PUBLIC "${CMAKE_SOURCE_DIR}/librariesEms")

# Link the game target against the engine library so that game can access engine headers and functions.
target_link_libraries(game PRIVATE engine)

# Apply the same WASM-specific options to the game if not building JS only.
if (NOT JS_ONLY)
    message(STATUS "Setting compilation target to WASM for Game")
    set_target_properties(game PROPERTIES OUTPUT_NAME "main")
    target_link_options(game PRIVATE
        "SHELL:-s USE_SDL=2"
        "SHELL:-s USE_SDL_IMAGE=2"
        "SHELL:-s USE_SDL_TTF=2"
        "SHELL:-s USE_SDL_MIXER=2"
        "SHELL:-s SDL2_IMAGE_FORMATS=[\"png\",\"jpg\",\"tga\"]"
        "SHELL:-s USE_WEBGL2=1"
        "SHELL:-s MIN_WEBGL_VERSION=2"
        "SHELL:-s MAX_WEBGL_VERSION=2"
        "SHELL:-s GL_PREINITIALIZED_CONTEXT=1"
        "SHELL:-s STACK_SIZE=500000"
        "SHELL:-s INITIAL_HEAP=83886080"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:-s NO_DISABLE_EXCEPTION_CATCHING"
        "SHELL:-s WASM=1"
        "SHELL:-s ASSERTIONS=1"
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/source/GameData@/GameData"
        "SHELL:-s GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=1"
        "SHELL:-lidbfs.js"
    )
endif()
