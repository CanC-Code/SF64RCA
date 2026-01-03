// SDL_main.cpp
#include <SDL.h>
#include <SDL_main.h>
#include <android/log.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "nfd.h"

#define LOG_TAG "Starfox64Recompiled"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// -------------------------------
// Forward declarations
// -------------------------------
int GameLoop();
bool InitializeEngine(const std::vector<uint8_t>& romData);
extern "C" const char* SDL_AndroidGetInternalStoragePath();

// -------------------------------
// File utilities
// -------------------------------
static bool copy_file(const std::filesystem::path& src, const std::filesystem::path& dst) {
    std::ifstream in(src, std::ios::binary);
    if (!in) return false;
    std::ofstream out(dst, std::ios::binary);
    if (!out) return false;
    out << in.rdbuf();
    return true;
}

static bool copy_asset(AAssetManager* mgr, const char* assetPath, const std::filesystem::path& dest) {
    AAsset* asset = AAssetManager_open(mgr, assetPath, AASSET_MODE_STREAMING);
    if (!asset) return false;

    std::ofstream out(dest, std::ios::binary);
    if (!out) {
        AAsset_close(asset);
        return false;
    }

    char buf[4096];
    int bytesRead = 0;
    while ((bytesRead = AAsset_read(asset, buf, sizeof(buf))) > 0) {
        out.write(buf, bytesRead);
    }

    AAsset_close(asset);
    return true;
}

static void copy_assets_dir(AAssetManager* mgr, const std::string& assetDir, const std::filesystem::path& destDir) {
    std::filesystem::create_directories(destDir);

    AAssetDir* dir = AAssetManager_openDir(mgr, assetDir.c_str());
    if (!dir) {
        SDL_Log("Warning: Cannot open asset directory: %s", assetDir.c_str());
        return;
    }

    const char* fname = nullptr;
    while ((fname = AAssetDir_getNextFileName(dir)) != nullptr) {
        std::string assetPath = assetDir + "/" + fname;
        std::filesystem::path outPath = destDir / fname;

        if (!copy_asset(mgr, assetPath.c_str(), outPath)) {
            SDL_Log("Failed to copy asset: %s -> %s", assetPath.c_str(), outPath.string().c_str());
        } else {
            SDL_Log("Copied asset: %s -> %s", assetPath.c_str(), outPath.string().c_str());
        }
    }
    AAssetDir_close(dir);
}

static void show_error(const char* msg) {
    if (SDL_WasInit(SDL_INIT_VIDEO)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Starfox64 Error", msg, nullptr);
    }
    SDL_Log("ERROR: %s", msg);
}

// -------------------------------
// Android main entry
// -------------------------------
extern "C"
int SDL_main(int argc, char* argv[]) {
    LOGI("SDL_main starting...");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
        LOGE("SDL_Init failed: %s", SDL_GetError());
        return -1;
    }

    // Internal storage
    std::string internalDir(SDL_AndroidGetInternalStoragePath());
    if (internalDir.empty()) {
        show_error("Cannot access internal storage.");
        return -1;
    }
    std::filesystem::path internalPath(internalDir);

    // Copy bundled assets
    AAssetManager* mgr = SDL_AndroidGetAssetManager();
    if (mgr) {
        LOGI("Copying bundled assets to internal storage...");
        copy_assets_dir(mgr, "shaders", internalPath / "shaders");
        copy_assets_dir(mgr, "mods", internalPath / "mods");
        copy_assets_dir(mgr, "patches", internalPath / "patches");
    } else {
        LOGI("Warning: AssetManager unavailable, skipping asset copy.");
    }

    // Prompt user for ROM if missing
    std::filesystem::path romPath = internalPath / "Starfox64.z64";
    if (!std::filesystem::exists(romPath)) {
        LOGI("No ROM found, prompting user...");

        nfdchar_t* outPath = nullptr;
        nfdresult_t result = NFD_OpenDialog("z64,rom", nullptr, &outPath);

        if (result == NFD_OKAY) {
            LOGI("User selected ROM: %s", outPath);
            std::filesystem::path selected(outPath);

            if (!copy_file(selected, romPath)) {
                show_error("Failed to copy ROM to internal storage.");
                free(outPath);
                return -1;
            }
            LOGI("ROM copied to: %s", romPath.string().c_str());
            free(outPath);
        } else if (result == NFD_CANCEL) {
            show_error("ROM selection cancelled. Cannot continue.");
            return -1;
        } else {
            std::string err = "NFD error: ";
            err += NFD_GetError();
            show_error(err.c_str());
            return -1;
        }
    } else {
        LOGI("ROM already exists: %s", romPath.string().c_str());
    }

    // Load ROM into memory
    std::vector<uint8_t> romData;
    {
        std::ifstream romFile(romPath, std::ios::binary | std::ios::ate);
        if (!romFile) {
            show_error(("Failed to open ROM: " + romPath.string()).c_str());
            return -1;
        }
        auto size = romFile.tellg();
        romFile.seekg(0, std::ios::beg);
        romData.resize(size);
        romFile.read(reinterpret_cast<char*>(romData.data()), size);
    }

    // Initialize engine
    if (!InitializeEngine(romData)) {
        show_error("Failed to initialize engine.");
        return -1;
    }

    LOGI("Starting main game loop...");
    int ret = GameLoop();

    SDL_Quit();
    return ret;
}

// -------------------------------
// InitializeEngine.cpp content
// -------------------------------
bool LoadPatchesFromROM(const std::vector<uint8_t>& romData);
bool LoadNRMs(const std::filesystem::path& modDir);
bool CompileShaders(const std::filesystem::path& shaderDir);

bool InitializeEngine(const std::vector<uint8_t>& romData) {
    std::filesystem::path internalPath(SDL_AndroidGetInternalStoragePath());

    // 1. Load patches
    if (!LoadPatchesFromROM(romData)) {
        SDL_Log("Failed to load patches from ROM");
        return false;
    }

    // 2. Load NRM mods
    std::filesystem::path modDir = internalPath / "mods";
    if (!LoadNRMs(modDir)) {
        SDL_Log("Failed to load NRM mods from: %s", modDir.string().c_str());
        return false;
    }

    // 3. Compile shaders
    std::filesystem::path shaderDir = internalPath / "shaders";
    if (!CompileShaders(shaderDir)) {
        SDL_Log("Failed to compile shaders from: %s", shaderDir.string().c_str());
        return false;
    }

    SDL_Log("Engine initialized successfully");
    return true;
}