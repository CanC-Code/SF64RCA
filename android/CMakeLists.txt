cmake_minimum_required(VERSION 3.22.1)

project(Starfox64Recompiled LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Android-specific definitions
add_compile_definitions(
    ANDROID
    SDL_MAIN_HANDLED
    HLSL_CPU
)

# Find required Android libraries
find_library(log-lib log)
find_library(android-lib android)
find_library(z-lib z)
find_library(EGL-lib EGL)
find_library(GLESv3-lib GLESv3)

# Fetch SDL2 for Android
include(FetchContent)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    SDL2
    URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.30.3.zip
)
FetchContent_MakeAvailable(SDL2)

# Add subdirectories for dependencies
set(RT64_STATIC TRUE)
set(RT64_SDL_WINDOW_VULKAN TRUE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/rt64 ${CMAKE_BINARY_DIR}/rt64)

set(BUILD_SHARED_LIBS OFF)
set(LUNASVG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/lunasvg ${CMAKE_BINARY_DIR}/lunasvg)

set(RMLUI_SVG_PLUGIN ON CACHE BOOL "" FORCE)
set(RMLUI_TESTS_ENABLED OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/RmlUi ${CMAKE_BINARY_DIR}/RmlUi)
target_compile_definitions(rmlui_core PRIVATE LUNASVG_BUILD_STATIC)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/N64ModernRuntime ${CMAKE_BINARY_DIR}/N64ModernRuntime)

# RecompiledFuncs Library
add_library(RecompiledFuncs STATIC)
target_compile_options(RecompiledFuncs PRIVATE
    -fno-strict-aliasing
    -Wno-unused-variable
    -Wno-implicit-function-declaration
)
target_include_directories(RecompiledFuncs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/N64ModernRuntime/ultramodern/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/N64ModernRuntime/librecomp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/N64ModernRuntime/N64Recomp/include
)
file(GLOB FUNC_C_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../RecompiledFuncs/*.c)
file(GLOB FUNC_CXX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../RecompiledFuncs/*.cpp)
target_sources(RecompiledFuncs PRIVATE ${FUNC_C_SOURCES} ${FUNC_CXX_SOURCES})

# PatchesLib Library
add_library(PatchesLib STATIC)
target_compile_options(PatchesLib PRIVATE
    -fno-strict-aliasing
    -Wno-unused-variable
    -Wno-implicit-function-declaration
)
target_include_directories(PatchesLib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/N64ModernRuntime/ultramodern/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/N64ModernRuntime/librecomp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/N64ModernRuntime/N64Recomp/include
)
target_sources(PatchesLib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../RecompiledPatches/patches.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../RecompiledPatches/patches_bin.c
)

# Main shared library
add_library(starfox64recompiled SHARED)

# All source files
set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/main/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/main/support.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/main/register_overlays.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/main/register_patches.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/main/rt64_render_context.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/../src/game/input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/game/controls.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/game/config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/game/scene_table.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/game/debug.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/game/quicksaving.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/game/recomp_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/game/recomp_actor_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/game/recomp_data_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/game/rom_decompression.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ui_renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ui_state.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ui_launcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ui_config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ui_prompt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ui_config_sub_menu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ui_color_hack.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ui_rml_hacks.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ui_elements.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ui_mod_details_panel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ui_mod_installer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ui_mod_menu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ui_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ui_api_events.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ui_api_images.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ui_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/util/hsv.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/core/ui_context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/elements/ui_button.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/elements/ui_clickable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/elements/ui_container.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/elements/ui_element.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/elements/ui_image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/elements/ui_label.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/elements/ui_radio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/elements/ui_scroll_container.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/elements/ui_slider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/elements/ui_span.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/elements/ui_style.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/elements/ui_text_input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/elements/ui_toggle.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/../rsp/aspMain.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/RmlUi/Backends/RmlUi_Platform_SDL.cpp
)

# Android wrapper
list(APPEND SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/app/src/main/cpp/SDL_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/app/src/main/cpp/main_android_wrapper.cpp
)

target_sources(starfox64recompiled PRIVATE ${SOURCES})

# Include directories
target_include_directories(starfox64recompiled PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/N64ModernRuntime/N64Recomp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/concurrentqueue
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/GamepadMotionHelpers
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/RmlUi/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/RmlUi/Backends
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/rt64/src/contrib
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/rt64/src/contrib/hlslpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/rt64/src/contrib/dxc/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/rt64/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/rt64/src/rhi
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/rt64/src/render
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/SlotMap
)

# Link libraries
target_link_libraries(starfox64recompiled PRIVATE
    PatchesLib
    RecompiledFuncs
    librecomp
    ultramodern
    rt64
    RmlUi::Core
    RmlUi::Debugger
    lunasvg
    SDL2::SDL2-static
    ${log-lib}
    ${android-lib}
    ${z-lib}
    ${EGL-lib}
    ${GLESv3-lib}
)