#include "RendererSelector.h"
#include <SDL.h>
#include <android/log.h>

#define LOG_TAG "RendererSelector"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace SF64RCA {

RendererBackend SelectBestRenderer() {
    if (SDL_Vulkan_LoadLibrary(nullptr)) {
        LOGI("Renderer selected: Vulkan");
        return RendererBackend::Vulkan;
    }

    LOGI("Renderer selected: OpenGLES (fallback)");
    return RendererBackend::OpenGLES;
}

}