cmake_minimum_required(VERSION 3.18)

project(Starfox64Recompiled LANGUAGES C CXX)

# Minimum Android API
set(ANDROID_PLATFORM android-34 CACHE STRING "Minimum Android API level")

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define ANDROID macro for conditional compilation
add_compile_definitions(ANDROID)
add_compile_definitions(SDL_MAIN_HANDLED)

# SDL2 path
set(SDL2_DIR ${CMAKE_SOURCE_DIR}/../lib/SDL2)

# Include directories
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

# Combine sources
set(ALL_SRC ${MAIN_SRC} ${PATCH_SRC})

# Build shared library for Android
add_library(starfox64recompiled SHARED ${ALL_SRC})

# Link SDL2 static library
add_library(SDL2 STATIC IMPORTED)
set_target_properties(SDL2 PROPERTIES
    IMPORTED_LOCATION "${SDL2_DIR}/lib/android/arm64-v8a/libSDL2.a"
)

target_link_libraries(starfox64recompiled
    SDL2
    log             # Android logging
    android         # Android system
    z               # zlib
)

# Include directories for NDK build
target_include_directories(starfox64recompiled PRIVATE
    ${SDL2_DIR}/include
)

# --- Skip host-only steps for Android ---
# vertex/pixel shader compilation, file_to_c, N64Recomp, etc.
# They must be pre-generated on host or GitHub Actions runner