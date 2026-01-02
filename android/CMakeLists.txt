cmake_minimum_required(VERSION 3.18)

# Minimum Android API for NDK
set(ANDROID_PLATFORM android-34 CACHE STRING "Minimum Android API level")

# Project name
project(Starfox64Recompiled LANGUAGES C CXX)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL2 path
set(SDL2_DIR ${CMAKE_SOURCE_DIR}/../lib/SDL2)

# Add include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/../include
    ${CMAKE_SOURCE_DIR}/../lib
    ${SDL2_DIR}/include
)

# Gather all source files
file(GLOB_RECURSE MAIN_SRC
    "${CMAKE_SOURCE_DIR}/../src/main/*.cpp"
    "${CMAKE_SOURCE_DIR}/../src/game/*.cpp"
    "${CMAKE_SOURCE_DIR}/../src/ui/*.cpp"
)

file(GLOB_RECURSE PATCH_SRC
    "${CMAKE_SOURCE_DIR}/../patches/*.c"
    "${CMAKE_SOURCE_DIR}/../patches/*.cpp"
)

# Add shared library target
add_library(starfox64recompiled SHARED
    ${MAIN_SRC}
    ${PATCH_SRC}
)

# Link SDL2 library
add_library(SDL2 STATIC IMPORTED)
set_target_properties(SDL2 PROPERTIES
    IMPORTED_LOCATION "${SDL2_DIR}/lib/android/arm64-v8a/libSDL2.a"
)

# Link libraries
target_link_libraries(starfox64recompiled
    SDL2
    log             # Android logging
    android         # Android system
    z               # zlib if needed
)

# Include SDL2 headers
target_include_directories(starfox64recompiled PRIVATE
    ${SDL2_DIR}/include
)

# Define preprocessor macros
target_compile_definitions(starfox64recompiled PRIVATE
    SDL_MAIN_HANDLED
    ANDROID
)