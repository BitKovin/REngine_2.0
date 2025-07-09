cmake_minimum_required(VERSION 3.16)
project(MyProject)

# ——————————————————————————————————————————————————————————
# Global settings
# ——————————————————————————————————————————————————————————
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build")

# Add Emscripten-specific compile options here (applies to all targets)
if (EMSCRIPTEN)
    add_compile_options(-O3 -DNDEBUG) #-pthread 
else()
    add_compile_options(-O3 -DNDEBUG)
endif()

# ——————————————————————————————————————————————————————————
# Platform detection
# ——————————————————————————————————————————————————————————
if (CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    set(EMSCRIPTEN TRUE)
    message(STATUS "Building for Emscripten")
    include_directories(SYSTEM ${EMSCRIPTEN_SYSROOT}/include)

elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(LINUX TRUE)
    message(STATUS "Building for Linux")

    # Imported Assimp static library
    add_library(Assimp::Assimp STATIC IMPORTED GLOBAL)
    set_target_properties(Assimp::Assimp PROPERTIES
        IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Lib/Linux/libassimp.a"
    )

    # Imported Jolt static library
    add_library(Jolt::Jolt STATIC IMPORTED GLOBAL)
    set_target_properties(Jolt::Jolt PROPERTIES
        IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Lib/Linux/libJolt.a"
    )
    
    add_library(FMOD::Core SHARED IMPORTED GLOBAL)
    set_target_properties(FMOD::Core PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Lib/Linux/libfmod.so"
    )

    add_library(FMOD::Studio SHARED IMPORTED GLOBAL)
    set_target_properties(FMOD::Studio PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Lib/Linux/libfmodstudio.so"
    )
    
add_library(mimalloc::mimalloc STATIC IMPORTED GLOBAL)
set_target_properties(mimalloc::mimalloc PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Lib/Linux/libmimalloc.a"
)

    

    # Find other Linux dependencies
    find_package(SDL2       REQUIRED)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_ttf   REQUIRED)
    find_package(GLEW       REQUIRED)
    find_package(OpenGL     REQUIRED)
    find_package(OpenAL     REQUIRED)
    find_package(ZLIB       REQUIRED)
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

add_library(engine STATIC ${ENGINE_SOURCES})
target_include_directories(engine 
    PUBLIC
      "${CMAKE_SOURCE_DIR}/source/Engine"   
)

if (EMSCRIPTEN)
    # Only list the .a files here; DO NOT apply Emscripten flags to engine
    target_link_libraries(engine PRIVATE
        "${CMAKE_SOURCE_DIR}/libsEms/libassimp.a"
        "${CMAKE_SOURCE_DIR}/libsEms/libIrrXML.a"
        "${CMAKE_SOURCE_DIR}/libsEms/libJolt.a"
        "${CMAKE_SOURCE_DIR}/libsEms/fmod_wasm.a"
        "${CMAKE_SOURCE_DIR}/libsEms/fmodstudio_wasm.a"
    )

elseif (LINUX)
    # Link OpenAL via imported target
    target_link_libraries(engine PRIVATE OpenAL::OpenAL)

	target_include_directories(engine 
    PUBLIC
      "${CMAKE_SOURCE_DIR}/libraries"     
	)

    # Link the rest as before
    target_link_libraries(engine PRIVATE
        Assimp::Assimp
        Jolt::Jolt
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_ttf::SDL2_ttf
        GLEW::GLEW
        OpenGL::GL
        ZLIB::ZLIB
	FMOD::Core
        FMOD::Studio 
        mimalloc::mimalloc 
    )
endif()

# ——————————————————————————————————————————————————————————
# Common WASM linker flags (for the final module)
# ——————————————————————————————————————————————————————————
set(WASM_COMMON_LINK_OPTIONS
    "SHELL:-s USE_SDL=2"
    "SHELL:-s USE_SDL_IMAGE=2"
    "SHELL:-s USE_SDL_TTF=2"
    "SHELL:-s SDL2_IMAGE_FORMATS=[\"png\",\"jpg\",\"tga\"]"
    "SHELL:-s USE_WEBGL2=1"
    "SHELL:-s MIN_WEBGL_VERSION=2"
    "SHELL:-s MAX_WEBGL_VERSION=2"
    "SHELL:-s GL_PREINITIALIZED_CONTEXT=1"
    "SHELL:-s STACK_SIZE=500000"
    "SHELL:-s INITIAL_HEAP=83886080"
    "SHELL:-s ALLOW_MEMORY_GROWTH=1"
    #"SHELL:-s TOTAL_MEMORY=134217728"
    "SHELL:-s NO_DISABLE_EXCEPTION_CATCHING"
    "SHELL:-s WASM=1"
    "SHELL:-s ASSERTIONS=1"
    "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/source/GameData@/GameData"
    "SHELL:-s GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=1"
    "SHELL:-lidbfs.js"
    "SHELL:-msimd128"
    "SHELL:-s EXPORTED_RUNTIME_METHODS=['cwrap','setValue','getValue']"
    #"SHELL:-pthread"
    #"SHELL:-s USE_PTHREADS=1"
    #"SHELL:-s PTHREAD_POOL_SIZE=32"
    #"SHELL:-s PTHREAD_POOL_SIZE_STRICT=1"
    #"SHELL:-s PROXY_TO_PTHREAD"
)

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

if (EMSCRIPTEN)
    message(STATUS "Applying WASM linker flags to final 'game' target")
    set_target_properties(game PROPERTIES OUTPUT_NAME "main")
    target_link_options(game PRIVATE ${WASM_COMMON_LINK_OPTIONS})
endif()

if (LINUX)
    target_link_options(game PRIVATE -pthread -ldl)
    install(TARGETS game DESTINATION bin)
endif()
