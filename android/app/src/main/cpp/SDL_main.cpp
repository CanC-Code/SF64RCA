#include <SDL.h>
#include <SDL_main.h>
#include <android/log.h>
#include <filesystem>
#include <vector>
#include <fstream>

#include "AndroidRT64Wrapper.h"

#define LOG_TAG "SF64RCA"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern "C" const char* SDL_AndroidGetInternalStoragePath();
extern "C" AAssetManager* SDL_AndroidGetAssetManager();

// --------------------------------------------------
// Global RT64 wrapper
// --------------------------------------------------
static SF64RCA::AndroidRT64Wrapper* rt64Wrapper = nullptr;

// --------------------------------------------------
// SDL entry
// --------------------------------------------------
extern "C"
int SDL_main(int argc, char* argv[]) {
    LOGI("SDL_main starting");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) {
        LOGE("SDL_Init failed: %s", SDL_GetError());
        return -1;
    }

    // Vulkan-first window (NO OpenGL flag)
    SDL_Window* window = SDL_CreateWindow(
        "Starfox64 Recompiled",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280,
        720,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        LOGE("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // --------------------------------------------------
    // RT64 init
    // --------------------------------------------------
    rt64Wrapper = new SF64RCA::AndroidRT64Wrapper();

    if (!rt64Wrapper->initialize(window, 1280, 720)) {
        LOGE("RT64 initialization failed");
        delete rt64Wrapper;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // --------------------------------------------------
    // Main loop
    // --------------------------------------------------
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        rt64Wrapper->resize(
                            event.window.data1,
                            event.window.data2
                        );
                    }
                    break;
            }
        }

        rt64Wrapper->renderFrame();
        SDL_Delay(1);
    }

    // --------------------------------------------------
    // Shutdown
    // --------------------------------------------------
    rt64Wrapper->shutdown();
    delete rt64Wrapper;
    rt64Wrapper = nullptr;

    SDL_DestroyWindow(window);
    SDL_Quit();

    LOGI("SDL_main exit clean");
    return 0;
}