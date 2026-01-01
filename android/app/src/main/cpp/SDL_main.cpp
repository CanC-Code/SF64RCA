#include <SDL.h>
#include <jni.h>
#include <android/log.h>

#define LOG_TAG "Starfox64Recompiled"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

// Forward declare your original main function
extern int StarfoxMain(int argc, char** argv);

// SDL_main replacement
int main(int argc, char* argv[]) {
    LOGI("SDL_main starting Starfox64Recompiled");

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) {
        LOGE("SDL_Init Error: %s", SDL_GetError());
        return 1;
    }

    // Call the original main function
    int result = StarfoxMain(argc, argv);

    SDL_Quit();
    return result;
}

// JNI bridge so Android can launch the app
extern "C" JNIEXPORT void JNICALL
Java_com_canc_starfox64_Starfox64Activity_nativeStart(JNIEnv* env, jobject thiz) {
    LOGI("nativeStart called from Java");

    char* argv[] = {const_cast<char*>("Starfox64Recompiled")};
    StarfoxMain(1, argv);
}