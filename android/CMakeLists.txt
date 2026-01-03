cmake_minimum_required(VERSION 3.22.1)

project(Starfox64Recompiled LANGUAGES C CXX)

# -------------------------------
# Compiler settings
# -------------------------------
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_definitions(
    ANDROID
    SDL_MAIN_HANDLED
    HLSL_CPU
    RT64_STATIC
    RT64_SDL_WINDOW_VULKAN
    DEFER_ROM_PATCHES
)

# -------------------------------
# Android NDK settings
# -------------------------------
set(CMAKE_ANDROID_STL_TYPE c++_shared)
if(NOT DEFINED CMAKE_ANDROID_ARCH_ABI)
    set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
endif()

# -------------------------------
# System libraries
# -------------------------------
find_library(log-lib log)
find_library(android-lib android)
find_library(z-lib z)

# -------------------------------
# Fetch SDL2 if not available
# -------------------------------
if(NOT TARGET SDL2::SDL2)
    include(FetchContent)
    FetchContent_Declare(
        SDL2
        URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.30.3.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(SDL2)
endif()

# -------------------------------
# Disable NativeFileDialog for Android
# -------------------------------
set(BUILD_NFD OFF CACHE BOOL "" FORCE)
set(NFD_PORTAL OFF CACHE BOOL "" FORCE)

# -------------------------------
# External libraries
# -------------------------------
# Add rt64 AFTER BUILD_NFD OFF is set
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../lib/rt64 ${CMAKE_BINARY_DIR}/rt64)
set(BUILD_SHARED_LIBS OFF)

# Lunasvg
set(LUNASVG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../lib/lunasvg)

# RmlUi
set(RMLUI_SVG_PLUGIN ON CACHE BOOL "" FORCE)
set(RMLUI_TESTS_ENABLED OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../lib/RmlUi)
target_compile_definitions(rmlui_core PRIVATE LUNASVG_BUILD_STATIC)

# N64ModernRuntime
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../lib/N64ModernRuntime)

# -------------------------------
# RecompiledFuncs Library
# -------------------------------
file(GLOB FUNC_C_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../../RecompiledFuncs/*.c)
file(GLOB FUNC_CXX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../../RecompiledFuncs/*.cpp)

add_library(RecompiledFuncs STATIC ${FUNC_C_SOURCES} ${FUNC_CXX_SOURCES})
target_compile_options(RecompiledFuncs PRIVATE
    -fno-strict-aliasing
    -Wno-unused-variable
    -Wno-implicit-function-declaration
)
target_include_directories(RecompiledFuncs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/N64ModernRuntime/ultramodern/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/N64ModernRuntime/librecomp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/N64ModernRuntime/N64Recomp/include
)

# -------------------------------
# PatchesLib Library
# -------------------------------
add_library(PatchesLib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../RecompiledPatches/patches.c
)
target_compile_options(PatchesLib PRIVATE
    -fno-strict-aliasing
    -Wno-unused-variable
    -Wno-implicit-function-declaration
)
target_include_directories(PatchesLib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/N64ModernRuntime/ultramodern/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/N64ModernRuntime/librecomp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/N64ModernRuntime/N64Recomp/include
)

# -------------------------------
# Main game sources
# -------------------------------
file(GLOB_RECURSE GAME_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/game/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/platform/android/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../rsp/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/RmlUi/Backends/*.cpp
)

# -------------------------------
# Automatically include Gradle-generated C files
# -------------------------------
set(GENERATED_SOURCES "")
set(GENERATED_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/../build/generated/cshaders"
    "${CMAKE_CURRENT_SOURCE_DIR}/../build/generated/cmods"
    "${CMAKE_CURRENT_SOURCE_DIR}/../build/generated/cpatches"
)

foreach(dir ${GENERATED_DIRS})
    if(EXISTS ${dir})
        file(GLOB GENERATED_IN_DIR "${dir}/*.c")
        list(APPEND GENERATED_SOURCES ${GENERATED_IN_DIR})
    endif()
endforeach()

# -------------------------------
# Main shared library
# -------------------------------
add_library(starfox64recompiled SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/app/src/main/cpp/SDL_main.cpp
    ${GAME_SOURCES}
    ${GENERATED_SOURCES}
)

# -------------------------------
# Include directories
# -------------------------------
target_include_directories(starfox64recompiled PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/N64ModernRuntime/N64Recomp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/concurrentqueue
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/GamepadMotionHelpers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/RmlUi/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/RmlUi/Backends
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/rt64/src/contrib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/rt64/src/contrib/hlslpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/rt64/src/contrib/dxc/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/rt64/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/rt64/src/rhi
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/rt64/src/render
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/freetype-windows-binaries/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/SlotMap
    ${CMAKE_BINARY_DIR}/shaders
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/src/main/cshaders
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/src/main/cmods
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/src/main/cpatches
)

# -------------------------------
# Compiler options
# -------------------------------
target_compile_options(starfox64recompiled PRIVATE
    -fno-strict-aliasing
    -Wno-unused-variable
    -Wno-implicit-function-declaration
)

# -------------------------------
# Link libraries
# -------------------------------
target_link_libraries(starfox64recompiled
    RecompiledFuncs
    PatchesLib
    librecomp
    ultramodern
    rt64
    RmlUi::Core
    RmlUi::Debugger
    lunasvg
    SDL2::SDL2
    ${log-lib}
    ${android-lib}
    ${z-lib}
)

# -------------------------------
# Output directory
# -------------------------------
set_target_properties(starfox64recompiled PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)