#include "AndroidRT64Wrapper.h"

#include <SDL.h>
#include <SDL_vulkan.h>

#include <rt64_render_device.h>
#include <rt64_render_context.h>

#include <android/log.h>

#define LOG_TAG "AndroidRT64Wrapper"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace SF64RCA {

AndroidRT64Wrapper::AndroidRT64Wrapper() = default;

AndroidRT64Wrapper::~AndroidRT64Wrapper() {
    shutdown();
}

bool AndroidRT64Wrapper::initialize(SDL_Window* window, int width, int height) {
    if (initialized) {
        return true;
    }

    if (!window) {
        LOGE("SDL_Window is null");
        return false;
    }

    sdlWindow = window;
    surfaceWidth = width;
    surfaceHeight = height;

    // Create RT64 render device (Vulkan via SDL)
    RT64::RenderDevice::CreateInfo deviceInfo{};
    deviceInfo.sdlWindow = sdlWindow;

    renderDevice = RT64::RenderDevice::Create(deviceInfo);
    if (!renderDevice) {
        LOGE("Failed to create RT64 RenderDevice");
        return false;
    }

    // Create render context
    RT64::RenderContext::CreateInfo contextInfo{};
    contextInfo.device = renderDevice;
    contextInfo.width = surfaceWidth;
    contextInfo.height = surfaceHeight;

    renderContext = RT64::RenderContext::Create(contextInfo);
    if (!renderContext) {
        LOGE("Failed to create RT64 RenderContext");
        renderDevice->Destroy();
        renderDevice = nullptr;
        return false;
    }

    initialized = true;
    LOGI("RT64 initialized (%dx%d)", width, height);
    return true;
}

void AndroidRT64Wrapper::resize(int width, int height) {
    if (!initialized || !renderContext) {
        return;
    }

    if (width == surfaceWidth && height == surfaceHeight) {
        return;
    }

    surfaceWidth = width;
    surfaceHeight = height;

    renderContext->Resize(width, height);
}

void AndroidRT64Wrapper::renderFrame() {
    if (!initialized || !renderContext) {
        return;
    }

    renderContext->BeginFrame();

    // NOTE:
    // Actual draw submission will come from the SF64RCA renderer
    // via RT64 command lists. This is intentionally empty for now.

    renderContext->EndFrame();
}

void AndroidRT64Wrapper::shutdown() {
    if (!initialized) {
        return;
    }

    if (renderContext) {
        renderContext->Destroy();
        renderContext = nullptr;
    }

    if (renderDevice) {
        renderDevice->Destroy();
        renderDevice = nullptr;
    }

    sdlWindow = nullptr;
    initialized = false;

    LOGI("RT64 shutdown complete");
}

bool AndroidRT64Wrapper::isInitialized() const {
    return initialized;
}

} // namespace SF64RCA