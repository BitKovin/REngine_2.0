cmake_minimum_required(VERSION 3.16)
project(MyProject)

# ——————————————————————————————————————————————————————————
# Global settings
# ——————————————————————————————————————————————————————————
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# All binaries & libraries end up under Build/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build")

option(JS_ONLY "Compiles to native JS (No WASM)" OFF)

# ——————————————————————————————————————————————————————————
# Platform detection
# ——————————————————————————————————————————————————————————
if(CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    set(EMSCRIPTEN TRUE)
    message(STATUS "Building for Emscripten")
    include_directories(SYSTEM ${EMSCRIPTEN_SYSROOT}/include)

elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(LINUX TRUE)
    message(STATUS "Building for Linux")

    # pkg-config for OpenAL Soft and Assimp
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(OPENAL REQUIRED openal)
    pkg_check_modules(ASSIMP REQUIRED assimp)

    # SDL2, image, ttf, GLEW, OpenGL via CMake Configs
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_ttf REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(OpenGL REQUIRED)

    # Imported Jolt static library
    add_library(Jolt::Jolt STATIC IMPORTED GLOBAL)
    set_target_properties(Jolt::Jolt PROPERTIES
        IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Lib/Linux/libJolt.a"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/libraries/Jolt/"
    )
endif()

# ——————————————————————————————————————————————————————————
# Common includes
# ——————————————————————————————————————————————————————————
include_directories(
    "${CMAKE_SOURCE_DIR}/librariesEms"
    "${CMAKE_SOURCE_DIR}/source/Engine"
)

# ——————————————————————————————————————————————————————————
# Engine library
# ——————————————————————————————————————————————————————————
file(GLOB_RECURSE ENGINE_SOURCES "${CMAKE_SOURCE_DIR}/source/Engine/*.cpp")
set(ENGINE_SRC "")
foreach(src IN LISTS ENGINE_SOURCES)
    if(NOT src MATCHES "/glm/")
        list(APPEND ENGINE_SRC ${src})
    endif()
endforeach()

add_library(engine STATIC ${ENGINE_SRC})

if(EMSCRIPTEN)
    # Link Emscripten‐built dependencies
    target_link_libraries(engine PRIVATE
        "${CMAKE_SOURCE_DIR}/libsEms/libassimp.a"
        "${CMAKE_SOURCE_DIR}/libsEms/libIrrXML.a"
        "${CMAKE_SOURCE_DIR}/libsEms/libJolt.a"
    )

elseif(LINUX)
    # Include & link OpenAL Soft and Assimp
    target_include_directories(engine PRIVATE
        ${OPENAL_INCLUDE_DIRS}
        ${ASSIMP_INCLUDE_DIRS}
    )
    target_link_libraries(engine PRIVATE
        ${OPENAL_LIBRARIES}
        ${ASSIMP_LIBRARIES}
        Jolt::Jolt
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_ttf::SDL2_ttf
        GLEW::GLEW
        OpenGL::GL
    )
endif()

# ——————————————————————————————————————————————————————————
# Common WASM linker flags
# ——————————————————————————————————————————————————————————
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
    "SHELL:-lidbfs.js"
)

if(EMSCRIPTEN AND NOT JS_ONLY)
    target_link_options(engine PRIVATE ${WASM_COMMON_LINK_OPTIONS})
endif()

# ——————————————————————————————————————————————————————————
# Game executable
# ——————————————————————————————————————————————————————————
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
    target_link_options(game PRIVATE -pthread -ldl)
    install(TARGETS game DESTINATION bin)
endif()

