cmake_minimum_required(VERSION 3.20)

# Project
project(Starfox64Recompiled LANGUAGES C CXX)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Minimum Android API
if(NOT DEFINED ANDROID_PLATFORM)
    set(ANDROID_PLATFORM android-33 CACHE STRING "Minimum Android API level")
endif()

# Root path
set(ROOT_DIR ${CMAKE_SOURCE_DIR}/..)

# ----- Add subprojects -----
add_subdirectory(${ROOT_DIR}/lib/rt64 ${CMAKE_BINARY_DIR}/rt64)
SET(LUNASVG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${ROOT_DIR}/lib/lunasvg)
SET(RMLUI_SVG_PLUGIN ON CACHE BOOL "" FORCE)
SET(RMLUI_TESTS_ENABLED OFF CACHE BOOL "" FORCE)
add_subdirectory(${ROOT_DIR}/lib/RmlUi)
target_compile_definitions(rmlui_core PRIVATE LUNASVG_BUILD_STATIC)
add_subdirectory(${ROOT_DIR}/lib/N64ModernRuntime)

# ----- RecompiledFuncs Library -----
add_library(RecompiledFuncs STATIC)
target_compile_options(RecompiledFuncs PRIVATE
    -fno-strict-aliasing
    -Wno-unused-variable
    -Wno-implicit-function-declaration
)
target_include_directories(RecompiledFuncs PRIVATE
    ${ROOT_DIR}/include
    ${ROOT_DIR}/lib/N64ModernRuntime/ultramodern/include
    ${ROOT_DIR}/lib/N64ModernRuntime/librecomp/include
    ${ROOT_DIR}/lib/N64ModernRuntime/N64Recomp/include
)
file(GLOB FUNC_C_SOURCES ${ROOT_DIR}/RecompiledFuncs/*.c)
file(GLOB FUNC_CXX_SOURCES ${ROOT_DIR}/RecompiledFuncs/*.cpp)
target_sources(RecompiledFuncs PRIVATE ${FUNC_C_SOURCES} ${FUNC_CXX_SOURCES})

# ----- PatchesLib Library -----
add_library(PatchesLib STATIC)
target_compile_options(PatchesLib PRIVATE
    -fno-strict-aliasing
    -Wno-unused-variable
    -Wno-implicit-function-declaration
)
target_include_directories(PatchesLib PRIVATE
    ${ROOT_DIR}/include
    ${ROOT_DIR}/lib/N64ModernRuntime/ultramodern/include
    ${ROOT_DIR}/lib/N64ModernRuntime/librecomp/include
    ${ROOT_DIR}/lib/N64ModernRuntime/N64Recomp/include
)
target_sources(PatchesLib PRIVATE
    ${ROOT_DIR}/RecompiledPatches/patches.c
    ${ROOT_DIR}/RecompiledPatches/patches_bin.c
)

# ----- Gather source files -----
file(GLOB_RECURSE MAIN_SRC
    "${ROOT_DIR}/src/main/*.cpp"
    "${ROOT_DIR}/src/game/*.cpp"
    "${ROOT_DIR}/src/ui/*.cpp"
    "${ROOT_DIR}/rsp/*.cpp"
)

# ----- Main shared library for Android -----
add_library(starfox64recompiled SHARED ${MAIN_SRC})

# ----- Include directories -----
target_include_directories(starfox64recompiled PRIVATE
    ${ROOT_DIR}/include
    ${ROOT_DIR}/lib/N64ModernRuntime/N64Recomp/include
    ${ROOT_DIR}/lib/concurrentqueue
    ${ROOT_DIR}/lib/GamepadMotionHelpers
    ${ROOT_DIR}/lib/RmlUi/Include
    ${ROOT_DIR}/lib/RmlUi/Backends
    ${ROOT_DIR}/lib/rt64/src/contrib
    ${ROOT_DIR}/lib/rt64/src/contrib/hlslpp/include
    ${ROOT_DIR}/lib/rt64/src/contrib/dxc/inc
    ${ROOT_DIR}/lib/rt64/src
    ${ROOT_DIR}/lib/rt64/src/rhi
    ${ROOT_DIR}/lib/rt64/src/render
    ${ROOT_DIR}/lib/freetype-windows-binaries/include
    ${ROOT_DIR}/lib/rt64/src/contrib/nativefiledialog-extended/src/include
    ${ROOT_DIR}/lib/SlotMap
)

# ----- Android SDL2 -----
include(FetchContent)
FetchContent_Declare(
    SDL2
    URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.30.3.zip
)
FetchContent_MakeAvailable(SDL2)

target_include_directories(starfox64recompiled PRIVATE ${SDL2_SOURCE_DIR}/include)
target_link_libraries(starfox64recompiled PRIVATE SDL2 log android z)

# ----- Link all libraries -----
target_link_libraries(starfox64recompiled PRIVATE
    PatchesLib
    RecompiledFuncs
    librecomp
    ultramodern
    rt64
    RmlUi::Core
    RmlUi::Debugger
    nfd
    lunasvg
)

# ----- Preprocessor macros -----
target_compile_definitions(starfox64recompiled PRIVATE
    SDL_MAIN_HANDLED
    ANDROID
    HLSL_CPU
)