cmake_minimum_required(VERSION 3.22)

project(SF64RCA LANGUAGES C CXX)

# ------------------------------------------------------------
# Android / Toolchain sanity
# ------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Let Gradle control STL — do NOT fight it
# ANDROID_STL is passed as c++_static by Gradle

# ------------------------------------------------------------
# Paths
# ------------------------------------------------------------

set(ROOT_DIR ${CMAKE_SOURCE_DIR}/..)

set(SDL2_SOURCE_DIR        ${ROOT_DIR}/android/SDL2)
set(LUNASVG_SOURCE_DIR    ${ROOT_DIR}/lib/lunasvg)
set(RMLUI_SOURCE_DIR      ${ROOT_DIR}/lib/RmlUi)
set(N64MR_SOURCE_DIR      ${ROOT_DIR}/lib/N64ModernRuntime)
set(RT64_SOURCE_DIR       ${ROOT_DIR}/lib/rt64)

# Binary dirs (required for out-of-tree add_subdirectory)
set(SDL2_BINARY_DIR     ${CMAKE_BINARY_DIR}/SDL2)
set(LUNASVG_BINARY_DIR  ${CMAKE_BINARY_DIR}/lunasvg)
set(RMLUI_BINARY_DIR    ${CMAKE_BINARY_DIR}/rmlui)
set(N64MR_BINARY_DIR    ${CMAKE_BINARY_DIR}/n64mr)
set(RT64_BINARY_DIR     ${CMAKE_BINARY_DIR}/rt64)

# ------------------------------------------------------------
# SDL2 configuration (Android)
# ------------------------------------------------------------

set(SDL_SHARED ON  CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)

set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)

set(SDL_JOYSTICK ON CACHE BOOL "" FORCE)
set(SDL_HAPTIC   ON CACHE BOOL "" FORCE)
set(SDL_SENSOR   ON CACHE BOOL "" FORCE)

add_subdirectory(
    ${SDL2_SOURCE_DIR}
    ${SDL2_BINARY_DIR}
)

# ------------------------------------------------------------
# lunasvg
# ------------------------------------------------------------

set(LUNASVG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(LUNASVG_BUILD_TOOLS  OFF CACHE BOOL "" FORCE)
set(LUNASVG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(
    ${LUNASVG_SOURCE_DIR}
    ${LUNASVG_BINARY_DIR}
)

# ------------------------------------------------------------
# RmlUi
# ------------------------------------------------------------

set(RMLUI_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(RMLUI_BUILD_SAMPLES    OFF CACHE BOOL "" FORCE)
set(RMLUI_BUILD_TESTS      OFF CACHE BOOL "" FORCE)
set(RMLUI_BUILD_DOCS       OFF CACHE BOOL "" FORCE)

add_subdirectory(
    ${RMLUI_SOURCE_DIR}
    ${RMLUI_BINARY_DIR}
)

# ------------------------------------------------------------
# N64ModernRuntime
# ------------------------------------------------------------

add_subdirectory(
    ${N64MR_SOURCE_DIR}
    ${N64MR_BINARY_DIR}
)

# ------------------------------------------------------------
# RT64
# ------------------------------------------------------------

# Avoid empty include_directories() inside RT64
# by ensuring required paths exist
set(RT64_ENABLE_VULKAN ON CACHE BOOL "" FORCE)
set(RT64_ENABLE_D3D12  OFF CACHE BOOL "" FORCE)
set(RT64_ENABLE_METAL  OFF CACHE BOOL "" FORCE)

add_subdirectory(
    ${RT64_SOURCE_DIR}
    ${RT64_BINARY_DIR}
)

# ------------------------------------------------------------
# Main native library
# ------------------------------------------------------------

add_library(starfox64recompiled SHARED
    ../src/main/cpp/native-lib.cpp
)

target_include_directories(starfox64recompiled PRIVATE
    ${ROOT_DIR}/src
)

target_link_libraries(starfox64recompiled
    SDL2
    SDL2main

    lunasvg

    rmlui_core

    n64modernruntime

    rt64

    android
    log
    EGL
    GLESv2
)

# ------------------------------------------------------------
# Android-specific defines
# ------------------------------------------------------------

target_compile_definitions(starfox64recompiled PRIVATE
    ANDROID
    SF64_ANDROID
)