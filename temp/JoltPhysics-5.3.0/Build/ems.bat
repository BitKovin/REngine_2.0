@echo off
REM Check if the first argument is empty
if "%~1"=="" (
    set "BUILD_TYPE=Distribution"
) else (
    set "BUILD_TYPE=%~1"
    shift
)

set "BUILD_DIR=WASM_%BUILD_TYPE%"

echo Usage: cmake_windows_emscripten.bat [Configuration]
echo Possible configurations: Debug, Release, Distribution (default)
echo Generating Makefile for build type "%BUILD_TYPE%" in folder "%BUILD_DIR%"

REM Jolt Physics specific flags for Emscripten
set "JOLT_FLAGS="
if "%BUILD_TYPE%"=="Release" (
    set "JOLT_FLAGS=-DJPH_USE_WASM=ON -DJPH_USE_WASM_SIMD=ON"
) else if "%BUILD_TYPE%"=="Distribution" (
    set "JOLT_FLAGS=-DJPH_USE_WASM=ON -DJPH_USE_WASM_SIMD=ON"
)

REM Call cmake with the given parameters
cmake -S . -B "%BUILD_DIR%" -G "Unix Makefiles" ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_TOOLCHAIN_FILE=%EMSDK%\upstream\emscripten\cmake\Modules\Platform\Emscripten.cmake ^
    -DBUILD_SHARED_LIBS=OFF ^
    -DJPH_BUILD_EXAMPLES=OFF ^
    -DJPH_BUILD_TESTS=OFF ^
    -DDOUBLE_PRECISION=OFF ^
    %JOLT_FLAGS% ^
    -DCMAKE_CXX_FLAGS="-pthread" ^
    -DCMAKE_C_FLAGS="-pthread" ^
    -DCMAKE_EXE_LINKER_FLAGS="-pthread -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=8" ^
    %*

echo.
echo Compile by running "make -j %NUMBER_OF_PROCESSORS% && node UnitTests.js" in folder "%BUILD_DIR%"
pause