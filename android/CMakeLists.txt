cmake_minimum_required(VERSION 3.22)
project(Starfox64Recompiled LANGUAGES C CXX)

# -------------------------------
# Compiler standards
# -------------------------------
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -------------------------------
# Android NDK / ABI
# -------------------------------
if(NOT DEFINED CMAKE_ANDROID_ARCH_ABI)
    set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
endif()
set(CMAKE_ANDROID_STL_TYPE c++_shared)

# -------------------------------
# Output directories
# -------------------------------
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# -------------------------------
# Global compile definitions
# -------------------------------
add_compile_definitions(
    ANDROID
    SDL_MAIN_HANDLED
    HLSL_CPU
    RT64_STATIC
    RT64_SDL_WINDOW_VULKAN
    DEFER_ROM_PATCHES
)

# -------------------------------
# SDL2 for Android
# -------------------------------
set(SDL2_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../android/SDL2")

if(NOT EXISTS "${SDL2_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "SDL2 source not found at ${SDL2_SOURCE_DIR}")
endif()

# Ensure Android variables are set before adding SDL2
set(ANDROID_PLATFORM 33)  # adjust your target API if needed
set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_ANDROID_ARCH_ABI ${CMAKE_ANDROID_ARCH_ABI})
set(CMAKE_ANDROID_NDK $ENV{ANDROID_NDK})
set(CMAKE_ANDROID_STL_TYPE c++_shared)

# Build SDL2 as static to avoid multiple SDL_main conflicts
set(SDL_SHARED OFF CACHE BOOL "" FORCE)

# Add SDL2 subdirectory
add_subdirectory(${SDL2_SOURCE_DIR} SDL2_build)

# -------------------------------
# System libraries
# -------------------------------
find_library(log-lib log)
find_library(android-lib android)
find_library(z-lib z)
find_library(vulkan-lib vulkan)

# -------------------------------
# Disable NativeFileDialog
# -------------------------------
set(BUILD_NFD OFF CACHE BOOL "" FORCE)
set(NFD_PORTAL OFF CACHE BOOL "" FORCE)
set(NFD_BUILD OFF CACHE BOOL "" FORCE)

# -------------------------------
# External libraries
# -------------------------------
# RT64
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(${CMAKE_SOURCE_DIR}/../lib/rt64 ${CMAKE_BINARY_DIR}/rt64)

# Lunasvg
set(LUNASVG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/../lib/lunasvg)

# RmlUi
set(RMLUI_SVG_PLUGIN ON CACHE BOOL "" FORCE)
set(RMLUI_TESTS_ENABLED OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/../lib/RmlUi)
target_compile_definitions(rmlui_core PRIVATE LUNASVG_BUILD_STATIC)

# N64ModernRuntime
add_subdirectory(${CMAKE_SOURCE_DIR}/../lib/N64ModernRuntime)

# -------------------------------
# RecompiledFuncs Library
# -------------------------------
file(GLOB FUNC_C_SOURCES ${CMAKE_SOURCE_DIR}/../RecompiledFuncs/*.c)
file(GLOB FUNC_CXX_SOURCES ${CMAKE_SOURCE_DIR}/../RecompiledFuncs/*.cpp)

add_library(RecompiledFuncs STATIC ${FUNC_C_SOURCES} ${FUNC_CXX_SOURCES})
target_compile_options(RecompiledFuncs PRIVATE
    -fno-strict-aliasing
    -Wno-unused-variable
    -Wno-implicit-function-declaration
)
target_include_directories(RecompiledFuncs PRIVATE
    ${CMAKE_SOURCE_DIR}/../include
    ${CMAKE_SOURCE_DIR}/../lib/N64ModernRuntime/ultramodern/include
    ${CMAKE_SOURCE_DIR}/../lib/N64ModernRuntime/librecomp/include
    ${CMAKE_SOURCE_DIR}/../lib/N64ModernRuntime/N64Recomp/include
)

# -------------------------------
# Optional PatchesLib
# -------------------------------
if(EXISTS ${CMAKE_SOURCE_DIR}/../RecompiledPatches/patches.c)
    add_library(PatchesLib STATIC
        ${CMAKE_SOURCE_DIR}/../RecompiledPatches/patches.c
        ${CMAKE_SOURCE_DIR}/../RecompiledPatches/patches_bin.c
    )
    target_compile_options(PatchesLib PRIVATE
        -fno-strict-aliasing
        -Wno-unused-variable
        -Wno-implicit-function-declaration
    )
    target_include_directories(PatchesLib PRIVATE
        ${CMAKE_SOURCE_DIR}/../include
        ${CMAKE_SOURCE_DIR}/../lib/N64ModernRuntime/ultramodern/include
        ${CMAKE_SOURCE_DIR}/../lib/N64ModernRuntime/librecomp/include
        ${CMAKE_SOURCE_DIR}/../lib/N64ModernRuntime/N64Recomp/include
    )
endif()

# -------------------------------
# Main sources
# -------------------------------
file(GLOB_RECURSE GAME_SOURCES
    ${CMAKE_SOURCE_DIR}/../src/main/*.cpp
    ${CMAKE_SOURCE_DIR}/../src/game/*.cpp
    ${CMAKE_SOURCE_DIR}/../src/ui/*.cpp
    ${CMAKE_SOURCE_DIR}/../src/platform/android/*.cpp
    ${CMAKE_SOURCE_DIR}/../rsp/*.cpp
    ${CMAKE_SOURCE_DIR}/../lib/RmlUi/Backends/*.cpp
)

# Include Android SDL_main
list(APPEND GAME_SOURCES ${CMAKE_SOURCE_DIR}/../android/app/src/main/cpp/SDL_main.cpp)

# Generated C sources (shaders/mods/patches)
set(GENERATED_SOURCES "")
foreach(dir
    ${CMAKE_SOURCE_DIR}/../android/app/src/main/cshaders
    ${CMAKE_SOURCE_DIR}/../android/app/src/main/cmods
    ${CMAKE_SOURCE_DIR}/../android/app/src/main/cpatches
)
    if(EXISTS ${dir})
        file(GLOB _gen "${dir}/*.c")
        list(APPEND GENERATED_SOURCES ${_gen})
    endif()
endforeach()

# NRM mods
file(GLOB NRM_FILES "${CMAKE_SOURCE_DIR}/../mods/*.nrm")
foreach(NRM_FILE ${NRM_FILES})
    get_filename_component(NRM_NAME ${NRM_FILE} NAME_WE)
    set(OUT_C "${CMAKE_CURRENT_BINARY_DIR}/mods/${NRM_NAME}.c")
    set(OUT_H "${CMAKE_CURRENT_BINARY_DIR}/mods/${NRM_NAME}.h")
    add_custom_command(
        OUTPUT ${OUT_C} ${OUT_H}
        COMMAND file_to_c ${NRM_FILE} ${NRM_NAME} ${OUT_C} ${OUT_H}
        DEPENDS ${NRM_FILE}
    )
    list(APPEND GENERATED_SOURCES ${OUT_C})
endforeach()

# -------------------------------
# Main shared library
# -------------------------------
add_library(starfox64recompiled SHARED
    ${GAME_SOURCES}
    ${GENERATED_SOURCES}
)

# -------------------------------
# Include directories
# -------------------------------
target_include_directories(starfox64recompiled PRIVATE
    ${CMAKE_SOURCE_DIR}/../include
    ${CMAKE_SOURCE_DIR}/../lib/N64ModernRuntime/N64Recomp/include
    ${CMAKE_SOURCE_DIR}/../lib/concurrentqueue
    ${CMAKE_SOURCE_DIR}/../lib/GamepadMotionHelpers
    ${CMAKE_SOURCE_DIR}/../lib/RmlUi/Include
    ${CMAKE_SOURCE_DIR}/../lib/RmlUi/Backends
    ${CMAKE_SOURCE_DIR}/../lib/SlotMap
    ${CMAKE_BINARY_DIR}
    ${SDL2_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/../lib/rt64/src/contrib
    ${CMAKE_SOURCE_DIR}/../lib/rt64/src/contrib/hlslpp/include
    ${CMAKE_SOURCE_DIR}/../lib/rt64/src/contrib/dxc/inc
    ${CMAKE_SOURCE_DIR}/../lib/rt64/src
    ${CMAKE_SOURCE_DIR}/../lib/rt64/src/rhi
    ${CMAKE_SOURCE_DIR}/../lib/rt64/src/render
    ${CMAKE_SOURCE_DIR}/../android/app/src/main/cshaders
    ${CMAKE_SOURCE_DIR}/../android/app/src/main/cmods
    ${CMAKE_SOURCE_DIR}/../android/app/src/main/cpatches
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
target_link_libraries(starfox64recompiled PRIVATE
    RecompiledFuncs
    rt64
    librecomp
    ultramodern
    RmlUi::Core
    RmlUi::Debugger
    lunasvg
    SDL2
    ${log-lib}
    ${android-lib}
    ${z-lib}
    ${vulkan-lib}
)

if(EXISTS ${CMAKE_SOURCE_DIR}/../RecompiledPatches/patches.c)
    target_link_libraries(starfox64recompiled PRIVATE PatchesLib)
endif()