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

    # 1) OpenAL Soft via pkg-config (prefers the soft implementation)
    find_package(PkgConfig REQUIRED)                    # loads pkg_check_modules :contentReference[oaicite:3]{index=3}
    pkg_check_modules(OPENAL REQUIRED openal-soft)      # locates openal-soft and defines OPENAL_INCLUDE_DIRS/LIBRARIES :contentReference[oaicite:4]{index=4}
    target_include_directories(engine PRIVATE ${OPENAL_INCLUDE_DIRS})
    target_link_libraries(engine PRIVATE PkgConfig::OPENAL)

    # Existing SDL2, SDL2_image, SDL2_ttf, GLEW, OpenGL linkage unchanged
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_ttf REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(OpenGL REQUIRED)

    target_link_libraries(engine PRIVATE
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_ttf::SDL2_ttf
        GLEW::GLEW
        OpenGL::GL
    )

    # 2) Assimp from package manager
    find_package(assimp REQUIRED)                        # loads AssimpConfig.cmake and creates assimp::assimp :contentReference[oaicite:5]{index=5}
    target_link_libraries(engine PRIVATE assimp::assimp)

    # 3) Custom Jolt library (imported from local libs folder)
    add_library(Jolt STATIC IMPORTED)
    set_target_properties(Jolt PROPERTIES
        IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/libs/libJolt.a"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/libs/include"
    )
    target_link_libraries(engine PRIVATE Jolt)

    # Optional: install target
    install(TARGETS game DESTINATION bin)
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