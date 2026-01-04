#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <string>
#include <vector>
#include <memory>
#include "RmlFileInterface_Android.h"
#include "RmlSystemInterface_Android.h"
#include "ui_renderer.h"
#include "SDL_main.h" // For rt64Wrapper access

#define LOG_TAG "SF64RCA"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Persistent static interfaces for RmlUi
static RmlFileInterface_Android* g_file_interface = nullptr;
static RmlSystemInterface_Android* g_system_interface = nullptr;
static recompui::RmlRenderInterface_RT64* g_rml_render_interface = nullptr;

// ------------------------------------------------------------
// Initialize RmlUi with Android AssetManager
// ------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL
Java_com_canc_starfox64_MainActivity_initRmlUi(JNIEnv* env, jobject thiz, jobject assetManager) {
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    if (!mgr) {
        LOGE("Failed to get AAssetManager from Java");
        return;
    }

    // Only create once
    if (!g_file_interface) g_file_interface = new RmlFileInterface_Android(mgr);
    if (!g_system_interface) g_system_interface = new RmlSystemInterface_Android();
    if (!g_rml_render_interface) g_rml_render_interface = new recompui::RmlRenderInterface_RT64();

    // Initialize render interface using RT64 wrapper
    if (!g_rml_render_interface->is_initialized()) {
        g_rml_render_interface->init(rt64Wrapper->getRenderInterface(), rt64Wrapper->getRenderDevice());
    }

    Rml::SetFileInterface(g_file_interface);
    Rml::SetSystemInterface(g_system_interface);
    Rml::SetRenderInterface(g_rml_render_interface->get_rml_interface());
    Rml::Initialise();

    LOGI("RmlUi initialized successfully on Android");
}

// ------------------------------------------------------------
// Helper: Load content URI into memory
// ------------------------------------------------------------
static std::vector<uint8_t> loadContentUri(JNIEnv* env, jobject context, jstring uri_str) {
    std::vector<uint8_t> data;

    const char* uri_cstr = env->GetStringUTFChars(uri_str, nullptr);
    if (!uri_cstr) return data;

    jclass contextClass = env->GetObjectClass(context);
    jmethodID getContentResolver = env->GetMethodID(contextClass, "getContentResolver", "()Landroid/content/ContentResolver;");
    jobject resolver = env->CallObjectMethod(context, getContentResolver);

    jclass resolverClass = env->GetObjectClass(resolver);
    jmethodID openInputStream = env->GetMethodID(resolverClass, "openInputStream", "(Landroid/net/Uri;)Ljava/io/InputStream;");

    jclass uriClass = env->FindClass("android/net/Uri");
    jmethodID parse = env->GetStaticMethodID(uriClass, "parse", "(Ljava/lang/String;)Landroid/net/Uri;");
    jobject uri = env->CallStaticObjectMethod(uriClass, parse, uri_str);

    jobject inputStream = env->CallObjectMethod(resolver, openInputStream, uri);
    if (!inputStream) {
        LOGE("Failed to open InputStream for URI");
        env->ReleaseStringUTFChars(uri_str, uri_cstr);
        return data;
    }

    // Read InputStream into a buffer
    jclass isClass = env->GetObjectClass(inputStream);
    jmethodID available = env->GetMethodID(isClass, "available", "()I");
    jmethodID read = env->GetMethodID(isClass, "read", "([B)I");
    jmethodID close = env->GetMethodID(isClass, "close", "()V");

    jint size = env->CallIntMethod(inputStream, available);
    if (size <= 0) size = 65536; // Fallback

    jbyteArray buffer = env->NewByteArray(size);
    jint bytesRead = 0;
    while ((bytesRead = env->CallIntMethod(inputStream, read, buffer)) > 0) {
        jbyte* buf = env->GetByteArrayElements(buffer, nullptr);
        data.insert(data.end(), buf, buf + bytesRead);
        env->ReleaseByteArrayElements(buffer, buf, 0);
    }

    env->CallVoidMethod(inputStream, close);
    env->DeleteLocalRef(buffer);

    env->ReleaseStringUTFChars(uri_str, uri_cstr);
    return data;
}

// ------------------------------------------------------------
// JNI: Load ROM from content URI
// ------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL
Java_com_canc_starfox64_MainActivity_pickRom(JNIEnv* env, jobject thiz, jstring uri_str) {
    if (!uri_str) {
        LOGE("pickRom: null URI");
        return;
    }

    std::vector<uint8_t> rom_data = loadContentUri(env, thiz, uri_str);
    if (rom_data.empty()) {
        LOGE("pickRom: failed to load ROM data");
        return;
    }

    LOGI("pickRom: loaded ROM of size %zu bytes", rom_data.size());

    // Pass ROM data to native emulator
    // Assume `rt64Wrapper->loadRomFromMemory` exists
    if (rt64Wrapper) {
        rt64Wrapper->loadRomFromMemory(rom_data.data(), rom_data.size());
        LOGI("ROM loaded into emulator");
    } else {
        LOGE("rt64Wrapper not initialized");
    }
}