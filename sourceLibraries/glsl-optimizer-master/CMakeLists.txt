cmake_minimum_required(VERSION 3.12)
project(GLSLOptimizer)  # Added a project name for better structure

include_directories(include)
include_directories(src/mesa)
include_directories(src/mapi)
include_directories(src/glsl)
include_directories(src)

option(DEBUG "Enable debugging" OFF)  # Changed default to OFF for consistency with CMake booleans

if(DEBUG)
    add_compile_options(-g -O0)
else()
    add_compile_options(-Os)
    add_compile_definitions(NDEBUG)
    add_link_options(-s)
endif()

file(GLOB glcpp-library_sources src/glsl/glcpp/*.c src/util/*.c)
#file(GLOB glcpp-library_sources_remove src/glsl/glcpp/glcpp.c)
#list(REMOVE_ITEM glcpp-library_sources ${glcpp-library_sources_remove})
add_library(glcpp-library ${glcpp-library_sources})

file(GLOB mesa_sources src/mesa/program/*.c src/mesa/main/*.c)
add_library(mesa ${mesa_sources})

file(GLOB glsl_sources src/glsl/*.cpp src/glsl/*.c)
file(GLOB glsl_sources_remove src/glsl/main.cpp src/glsl/builtin_stubs.cpp)
list(REMOVE_ITEM glsl_sources ${glsl_sources_remove})
add_library(glsl_optimizer ${glsl_sources})
target_link_libraries(glsl_optimizer glcpp-library mesa)

add_executable(glsl_compiler src/glsl/main.cpp)
target_link_libraries(glsl_compiler glsl_optimizer)

file(GLOB glsl_test_sources tests/*.cpp)
add_executable(glsl_test ${glsl_test_sources})
target_link_libraries(glsl_test glsl_optimizer)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # OSX-specific build requirements
    find_library(OpenGL_LIBRARY OpenGL)
    target_link_libraries(glsl_test ${OpenGL_LIBRARY})
endif()

file(GLOB glslopt_sources contrib/glslopt/*.cpp)
add_executable(glslopt ${glslopt_sources})
target_link_libraries(glslopt glsl_optimizer)

#add_executable(glcpp src/glsl/glcpp/glcpp.c)
#target_link_libraries(glcpp glsl_optimizer)