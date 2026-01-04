#include <SDL.h>
#include <SDL_main.h>
#include <android/log.h>
#include <filesystem>
#include <fstream>

#include "AndroidRT64Wrapper.h"

#define LOG_TAG "SF64RCA"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Global wrapper
static SF64RCA::AndroidRT64Wrapper* rt64Wrapper = nullptr;

// Helper: show error
static void show_error(const char* msg) {
    if (SDL_WasInit(SDL_INIT_VIDEO)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SF64RCA Error", msg, nullptr);
    }
    SDL_Log("ERROR: %s", msg);
}

// ------------------------
// SDL_main entry
// ------------------------
extern "C"
int SDL_main(int argc, char* argv[]) {
    LOGI("SDL_main starting...");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
        LOGE("SDL_Init failed: %s", SDL_GetError());
        return -1;
    }

    // Create SDL window
    SDL_Window* window = SDL_CreateWindow(
        "Starfox64 Recompiled",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        1280, 720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        LOGE("Failed to create SDL_Window: %s", SDL_GetError());
        return -1;
    }

    // Initialize RT64 wrapper with Vulkan
    rt64Wrapper = new SF64RCA::AndroidRT64Wrapper();
    if (!rt64Wrapper->initialize(window, 1280, 720, SF64RCA::RendererBackend::Vulkan)) {
        LOGE("Failed to initialize RT64 wrapper");
        return -1;
    }

    // Main loop
    SDL_Event event;
    bool running = true;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        int newWidth  = event.window.data1;
                        int newHeight = event.window.data2;

                        if (rt64Wrapper && rt64Wrapper->isInitialized()) {
                            rt64Wrapper->resize(newWidth, newHeight);
                            LOGI("RT64 resized to %dx%d", newWidth, newHeight);
                        }
                    }
                    break;

                default: break;
            }
        }

        if (rt64Wrapper && rt64Wrapper->isInitialized()) {
            rt64Wrapper->renderFrame();
        }

        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    if (rt64Wrapper) {
        rt64Wrapper->shutdown();
        delete rt64Wrapper;
        rt64Wrapper = nullptr;
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}