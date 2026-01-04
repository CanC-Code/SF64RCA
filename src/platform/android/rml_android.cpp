#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "RmlFileInterface_Android.h"
#include "RmlSystemInterface_Android.h"
#include "ui_renderer.h"
#include "SDL_main.h" // For rt64Wrapper access

extern "C" JNIEXPORT void JNICALL
Java_com_canc_starfox64_MainActivity_initRmlUi(JNIEnv* env, jobject thiz, jobject assetManager) {
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);

    static RmlFileInterface_Android file_interface(mgr);
    static RmlSystemInterface_Android system_interface;
    static recompui::RmlRenderInterface_RT64 rml_render_interface;

    // Initialize renderer using RT64 wrapper
    rml_render_interface.init(rt64Wrapper->getRenderInterface(), rt64Wrapper->getRenderDevice());

    Rml::SetFileInterface(&file_interface);
    Rml::SetSystemInterface(&system_interface);
    Rml::SetRenderInterface(rml_render_interface.get_rml_interface());
    Rml::Initialise();

    SDL_Log("RmlUi initialized successfully on Android");
}