cmake_minimum_required(VERSION 3.22)

# ------------------------------------------------------------
# Project
# ------------------------------------------------------------
project(sf64rca LANGUAGES C CXX)

# ------------------------------------------------------------
# Android / Toolchain sanity
# ------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Match Gradle: -DANDROID_STL=c++_static
set(CMAKE_ANDROID_STL_TYPE c++_static)

# Silence warnings from third-party code
add_compile_options(-Wno-unused-parameter -Wno-unused-variable)

# ------------------------------------------------------------
# Paths
# ------------------------------------------------------------
set(PROJECT_ROOT        ${CMAKE_CURRENT_LIST_DIR}/..)
set(ANDROID_ROOT        ${CMAKE_CURRENT_LIST_DIR})

set(SDL2_SOURCE_DIR     ${ANDROID_ROOT}/SDL2)
set(FREETYPE_SOURCE_DIR ${ANDROID_ROOT}/freetype)
set(RMLUI_SOURCE_DIR    ${PROJECT_ROOT}/RmlUi)

# ------------------------------------------------------------
# SDL2 (from source)
# ------------------------------------------------------------
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON  CACHE BOOL "" FORCE)
set(SDL_TEST OFF    CACHE BOOL "" FORCE)

add_subdirectory(
    ${SDL2_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/SDL2
)

# ------------------------------------------------------------
# FreeType (minimal)
# ------------------------------------------------------------
set(FT_DISABLE_ZLIB      ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2    ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG      ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI   ON CACHE BOOL "" FORCE)

set(FT_WITH_ZLIB      OFF CACHE BOOL "" FORCE)
set(FT_WITH_BZIP2    OFF CACHE BOOL "" FORCE)
set(FT_WITH_PNG      OFF CACHE BOOL "" FORCE)
set(FT_WITH_HARFBUZZ OFF CACHE BOOL "" FORCE)
set(FT_WITH_BROTLI   OFF CACHE BOOL "" FORCE)

add_subdirectory(
    ${FREETYPE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/freetype
)

if (NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype ALIAS freetype)
endif()

# ------------------------------------------------------------
# RmlUi (force FreeType usage, disable samples/tools/tests)
# ------------------------------------------------------------
set(RMLUI_BACKEND SDL2 CACHE STRING "" FORCE)
set(RMLUI_FONT_ENGINE freetype CACHE STRING "" FORCE)
set(RMLUI_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_BUILD_TESTS   OFF CACHE BOOL "" FORCE)
set(RMLUI_BUILD_TOOLS   OFF CACHE BOOL "" FORCE)

# Force RmlUi to use our FreeType, not system
set(FREETYPE_FOUND TRUE)
set(FREETYPE_INCLUDE_DIRS
    ${FREETYPE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/freetype/include
)
set(FREETYPE_LIBRARIES freetype)

add_subdirectory(
    ${RMLUI_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/RmlUi
)

# ------------------------------------------------------------
# Native game library
# ------------------------------------------------------------
file(GLOB_RECURSE GAME_SOURCES
    ${PROJECT_ROOT}/src/*.c
    ${PROJECT_ROOT}/src/*.cpp
)

add_library(sf64_native SHARED
    ${GAME_SOURCES}
)

# Include paths (including Android-specific headers)
target_include_directories(sf64_native PRIVATE
    ${PROJECT_ROOT}/src
    ${PROJECT_ROOT}/src/platform/android      # <-- Android-specific interfaces
    ${SDL2_SOURCE_DIR}/include
    ${FREETYPE_SOURCE_DIR}/include
    ${RMLUI_SOURCE_DIR}/Include
)

target_link_libraries(sf64_native
    SDL2-static
    freetype
    RmlCore
    RmlDebugger
    android
    log
    EGL
    GLESv3
)

# ------------------------------------------------------------
# Android log + SDL main
# ------------------------------------------------------------
target_compile_definitions(sf64_native PRIVATE
    ANDROID
    SDL_MAIN_HANDLED
)