#include <SDL.h>
#include <android/log.h>

#define LOG_TAG "Starfox64Recompiled"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// This is implemented in main_android_wrapper.cpp
extern int StarfoxMain(int argc, char** argv);

// SDL will call this function on Android
extern "C"
int SDL_main(int argc, char* argv[]) {
    LOGI("SDL_main started");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) {
        LOGE("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    int result = StarfoxMain(argc, argv);

    SDL_Quit();
    return result;
}