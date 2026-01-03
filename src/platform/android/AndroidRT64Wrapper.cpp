#include "AndroidRT64Wrapper.h"

#include <SDL.h>
#include <android/log.h>
#include "ultramodern/ultramodern.hpp"
#include "zelda_render.h"

#define LOG_TAG "AndroidRT64Wrapper"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace SF64RCA {

AndroidRT64Wrapper::AndroidRT64Wrapper() = default;

AndroidRT64Wrapper::~AndroidRT64Wrapper() {
    shutdown();
}

bool AndroidRT64Wrapper::initialize(SDL_Window* window, int width, int height) {
    if (initialized) return true;
    if (!window) {
        LOGE("SDL_Window is null");
        return false;
    }

    sdlWindow = window;
    surfaceWidth = width;
    surfaceHeight = height;

    ultramodern::renderer::WindowHandle window_handle{};
    window_handle.window = window;

    static uint8_t rdram[0x4000000]; // 64MB RDRAM
    renderContext = zelda64::renderer::create_render_context(rdram, window_handle, true);

    if (!renderContext) {
        LOGE("Failed to create RT64Context");
        return false;
    }

    // Set initial resolution in userConfig
    auto& config = ultramodern::renderer::get_graphics_config();
    renderContext->update_config(config, config);

    initialized = true;
    LOGI("RT64Context initialized (%dx%d)", width, height);
    return true;
}

void AndroidRT64Wrapper::resize(int width, int height) {
    if (!initialized || !renderContext) return;
    if (width == surfaceWidth && height == surfaceHeight) return;

    surfaceWidth = width;
    surfaceHeight = height;

    // Update userConfig resolution to match new swap chain size
    auto& config = ultramodern::renderer::get_graphics_config();
    config.manual_width = width;
    config.manual_height = height;

    renderContext->update_config(config, config);

    LOGI("RT64Context resized to %dx%d", width, height);
}

void AndroidRT64Wrapper::renderFrame() {
    if (!initialized || !renderContext) return;

    renderContext->check_texture_pack_actions();
    renderContext->update_screen();
}

void AndroidRT64Wrapper::shutdown() {
    if (!initialized) return;

    if (renderContext) {
        renderContext->shutdown();
        renderContext.reset();
    }

    sdlWindow = nullptr;
    initialized = false;

    LOGI("RT64Context shutdown complete");
}

bool AndroidRT64Wrapper::isInitialized() const {
    return initialized;
}

} // namespace SF64RCA