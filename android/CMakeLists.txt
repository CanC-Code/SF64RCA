cmake_minimum_required(VERSION 3.22)

# ------------------------------------------------------------
# Project
# ------------------------------------------------------------
project(sf64rca LANGUAGES C CXX)

# ------------------------------------------------------------
# Toolchain / Language
# ------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_ANDROID_STL_TYPE c++_static)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)  # Required for SHARED libs

add_compile_options(
    -Wno-unused-parameter
    -Wno-unused-variable
    -fno-strict-aliasing
    -Wno-implicit-function-declaration
)

# ------------------------------------------------------------
# Paths
# ------------------------------------------------------------
set(PROJECT_ROOT         ${CMAKE_CURRENT_LIST_DIR}/..)
set(ANDROID_ROOT         ${CMAKE_CURRENT_LIST_DIR})

set(SDL2_SOURCE_DIR      ${ANDROID_ROOT}/SDL2)
set(FREETYPE_SOURCE_DIR  ${ANDROID_ROOT}/freetype)
set(RMLUI_SOURCE_DIR     ${ANDROID_ROOT}/RmlUi)

# ------------------------------------------------------------
# SDL2 (static, Android-native)
# ------------------------------------------------------------
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON  CACHE BOOL "" FORCE)
set(SDL_TEST   OFF CACHE BOOL "" FORCE)

add_subdirectory(
    ${SDL2_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/SDL2
)

# ------------------------------------------------------------
# FreeType (static, minimal)
# ------------------------------------------------------------
set(FT_DISABLE_ZLIB     ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2    ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG      ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI   ON CACHE BOOL "" FORCE)

add_subdirectory(
    ${FREETYPE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/freetype
)

if(NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype ALIAS freetype)
endif()

# ------------------------------------------------------------
# RmlUi (SDL2 backend)
# ------------------------------------------------------------
set(RMLUI_BACKEND SDL2 CACHE STRING "" FORCE)
set(RMLUI_FONT_ENGINE freetype CACHE STRING "" FORCE)
set(RMLUI_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_BUILD_TESTS   OFF CACHE BOOL "" FORCE)
set(RMLUI_BUILD_TOOLS   OFF CACHE BOOL "" FORCE)

add_subdirectory(
    ${RMLUI_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/RmlUi
)

# Optional: enforce static Lunasvg plugin if SVGs are used
target_compile_definitions(RmlCore PRIVATE LUNASVG_BUILD_STATIC)

# ------------------------------------------------------------
# Vulkan detection (optional)
# ------------------------------------------------------------
find_package(Vulkan)
if(Vulkan_FOUND)
    message(STATUS "Vulkan detected: ${Vulkan_INCLUDE_DIR}")
    add_definitions(-DRT64_USE_VULKAN)
endif()

# ------------------------------------------------------------
# Native game sources
# ------------------------------------------------------------
file(GLOB_RECURSE GAME_SOURCES
    ${PROJECT_ROOT}/src/*.c
    ${PROJECT_ROOT}/src/*.cpp
    ${PROJECT_ROOT}/src/platform/android/rml_android.cpp
)

# ------------------------------------------------------------
# Include generated assets
# ------------------------------------------------------------
file(GLOB_RECURSE ASSET_SOURCES
    ${ANDROID_ROOT}/app/src/main/cmods/*.c
    ${ANDROID_ROOT}/app/src/main/cassets/*.c
    ${ANDROID_ROOT}/app/src/main/cshaders/*.c
    ${ANDROID_ROOT}/app/src/main/cpatches/*.c
)

# Flatten paths to avoid collisions (use only filename)
foreach(f ${ASSET_SOURCES})
    get_filename_component(fname ${f} NAME)
    list(APPEND FLATTENED_ASSETS ${f})
endforeach()
list(APPEND GAME_SOURCES ${FLATTENED_ASSETS})

# ------------------------------------------------------------
# Build shared library (JNI)
# ------------------------------------------------------------
add_library(starfox64recompiled SHARED ${GAME_SOURCES})

# Include directories
target_include_directories(starfox64recompiled PRIVATE
    ${PROJECT_ROOT}/src
    ${PROJECT_ROOT}/src/platform/android
    ${ANDROID_ROOT}/app/src/main/cmods
    ${ANDROID_ROOT}/app/src/main/cassets
    ${ANDROID_ROOT}/app/src/main/cshaders
    ${ANDROID_ROOT}/app/src/main/cpatches
    ${Vulkan_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(starfox64recompiled
    SDL2-static
    Freetype::Freetype
    RmlCore
    RmlDebugger
    log
    android
    EGL
    GLESv3
)

if(Vulkan_FOUND)
    target_link_libraries(starfox64recompiled Vulkan::Vulkan)
endif()

# ------------------------------------------------------------
# Android defines
# ------------------------------------------------------------
target_compile_definitions(starfox64recompiled PRIVATE
    ANDROID
    SDL_MAIN_HANDLED
)

# ------------------------------------------------------------
# Optional: output shared library to Gradle jniLibs
# ------------------------------------------------------------
set_target_properties(starfox64recompiled PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${ANDROID_ROOT}/app/src/main/jniLibs/${ANDROID_ABI}
)