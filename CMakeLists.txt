cmake_minimum_required(VERSION 3.16)
project(MyProject)

# Set C++ standard and global flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build")

option(JS_ONLY "Compiles to native JS (No WASM)" OFF)

# Platform detection
if(CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    set(EMSCRIPTEN TRUE)
    message(STATUS "Building for Emscripten")
    include_directories(SYSTEM ${EMSCRIPTEN_SYSROOT}/include)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(LINUX TRUE)
    message(STATUS "Building for Linux")
endif()

# Global include directories
include_directories(
    "${CMAKE_SOURCE_DIR}/librariesEms"
    "${CMAKE_SOURCE_DIR}/source/Engine"
)

###############################################################################
# Engine Setup
###############################################################################
file(GLOB_RECURSE ENGINE_SOURCES "${CMAKE_SOURCE_DIR}/source/Engine/*.cpp")
set(ENGINE_SRC "")
foreach(source_file IN LISTS ENGINE_SOURCES)
    if(NOT source_file MATCHES "/glm/")
        list(APPEND ENGINE_SRC ${source_file})
    endif()
endforeach()

add_library(engine STATIC ${ENGINE_SRC})

if(EMSCRIPTEN)
    target_link_libraries(engine PRIVATE
        "${CMAKE_SOURCE_DIR}/libsEms/libassimp.a"
        "${CMAKE_SOURCE_DIR}/libsEms/libIrrXML.a"
        "${CMAKE_SOURCE_DIR}/libsEms/libJolt.a"
    )
elseif(LINUX)
    find_package(OpenAL REQUIRED)
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_ttf REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(OpenGL REQUIRED)

    target_link_libraries(engine PRIVATE
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_ttf::SDL2_ttf
        OpenAL::OpenAL
        GLEW::GLEW
        OpenGL::GL
    )
endif()

# Common WASM linker options
set(WASM_COMMON_LINK_OPTIONS
    "SHELL:-s USE_SDL=2"
    "SHELL:-s USE_SDL_IMAGE=2"
    "SHELL:-s USE_SDL_TTF=2"
    "SHELL:-s SDL2_IMAGE_FORMATS=[\"png\",\"jpg\",\"tga\"]"
    "SHELL:-s USE_WEBGL2=1"
    "SHELL:-s GL_PREINITIALIZED_CONTEXT=1"
    "SHELL:-s STACK_SIZE=500000"
    "SHELL:-s INITIAL_HEAP=83886080"
    "SHELL:-s ALLOW_MEMORY_GROWTH=1"
    "SHELL:-s NO_DISABLE_EXCEPTION_CATCHING"
    "SHELL:-s WASM=1"
    "SHELL:-s ASSERTIONS=1"
    "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/source/GameData@/GameData"
    "SHELL:-lidbfs.js"  # Fixed IDBFS error
)

if(NOT JS_ONLY)
    target_link_options(engine PRIVATE ${WASM_COMMON_LINK_OPTIONS})
endif()

###############################################################################
# Game Setup
###############################################################################
file(GLOB_RECURSE GAME_SOURCES "${CMAKE_SOURCE_DIR}/source/Game/*.cpp")
add_executable(game ${GAME_SOURCES})

target_include_directories(game PUBLIC
    "${CMAKE_SOURCE_DIR}/librariesEms"
    "${CMAKE_SOURCE_DIR}/source/Engine"
)

target_link_libraries(game PRIVATE engine)

if(EMSCRIPTEN AND NOT JS_ONLY)
    target_link_options(game PRIVATE ${WASM_COMMON_LINK_OPTIONS})
    set_target_properties(game PROPERTIES OUTPUT_NAME "main")
endif()

if(LINUX)
    target_link_options(game PRIVATE
        -pthread
        -ldl
    )
    install(TARGETS game DESTINATION bin)
endif()