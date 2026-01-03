cmake_minimum_required(VERSION 3.18)

project(Starfox64Recompiled LANGUAGES C CXX)

set(ANDROID_PLATFORM android-34 CACHE STRING "Minimum Android API level")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(ANDROID SDL_MAIN_HANDLED)

# SDL2
set(SDL2_DIR ${CMAKE_SOURCE_DIR}/../lib/SDL2)
include_directories(${SDL2_DIR}/include)
add_library(SDL2 STATIC IMPORTED)
set_target_properties(SDL2 PROPERTIES
    IMPORTED_LOCATION "${SDL2_DIR}/lib/android/arm64-v8a/libSDL2.a"
)

# Include main sources
include_directories(
    ${CMAKE_SOURCE_DIR}/../include
    ${CMAKE_SOURCE_DIR}/../lib
    ${CMAKE_SOURCE_DIR}/../RecompiledPatches
    ${CMAKE_SOURCE_DIR}/../android/app/src/main/cmods
)

file(GLOB_RECURSE MAIN_SRC
    "${CMAKE_SOURCE_DIR}/../src/main/*.cpp"
    "${CMAKE_SOURCE_DIR}/../src/game/*.cpp"
    "${CMAKE_SOURCE_DIR}/../src/ui/*.cpp"
)

file(GLOB_RECURSE PATCH_SRC
    "${CMAKE_SOURCE_DIR}/../RecompiledPatches/*.c"
    "${CMAKE_SOURCE_DIR}/../RecompiledPatches/*.cpp"
)

file(GLOB_RECURSE MOD_SRC
    "${CMAKE_SOURCE_DIR}/../android/app/src/main/cmods/*.c"
)

set(ALL_SRC ${MAIN_SRC} ${PATCH_SRC} ${MOD_SRC})

add_library(starfox64recompiled SHARED ${ALL_SRC})

target_link_libraries(starfox64recompiled
    SDL2
    log
    android
    z
)

# Extra includes for NDK build
target_include_directories(starfox64recompiled PRIVATE
    ${SDL2_DIR}/include
)