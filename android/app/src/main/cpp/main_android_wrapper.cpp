#include <SDL.h>
#include <SDL_main.h>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem>
#include <cstdio>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "nfd.h"

// Forward declarations
int GameLoop(); // Your existing game loop
bool InitializeEngine(const std::vector<uint8_t>& romData); // Engine init (patches, overlays, mods, shaders)
extern "C" const char* SDL_AndroidGetInternalStoragePath(); // Provided by SDL

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

// Copy from AAssetManager to internal storage
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

// Recursively copy asset directory (simplified, assumes flat structure)
static void copy_assets_dir(AAssetManager* mgr, const char* assetDir, const std::filesystem::path& destDir) {
    std::filesystem::create_directories(destDir);

    AAssetDir* dir = AAssetManager_openDir(mgr, assetDir);
    const char* fname = nullptr;
    while ((fname = AAssetDir_getNextFileName(dir)) != nullptr) {
        std::string assetPath = std::string(assetDir) + "/" + fname;
        std::filesystem::path outPath = destDir / fname;
        copy_asset(mgr, assetPath.c_str(), outPath);
    }
    AAssetDir_close(dir);
}

// Show error via SDL dialog
static void show_error(const char* msg) {
    if (SDL_WasInit(SDL_INIT_VIDEO)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Starfox64 Error", msg, nullptr);
    }
    SDL_Log("ERROR: %s", msg);
}

// -------------------------------
// Android entry point
// -------------------------------
extern "C"
int StarfoxMain(int argc, char** argv) {
    // Initialize SDL minimally
    if (SDL_Init(0) < 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return -1;
    }

    // Get internal storage path
    std::string internalDir(SDL_AndroidGetInternalStoragePath());
    if (internalDir.empty()) {
        show_error("Cannot access internal storage.");
        return -1;
    }
    std::filesystem::path internalPath(internalDir);

    // -------------------------------
    // Copy bundled assets (shaders/mods/patches)
    // -------------------------------
    AAssetManager* mgr = SDL_AndroidGetJNIEnv() ? SDL_AndroidGetAssetManager() : nullptr;
    if (mgr) {
        SDL_Log("Copying bundled assets to internal storage...");
        copy_assets_dir(mgr, "shaders", internalPath / "shaders");
        copy_assets_dir(mgr, "mods", internalPath / "mods");
        copy_assets_dir(mgr, "patches", internalPath / "patches");
    } else {
        SDL_Log("No AssetManager available, skipping asset copy.");
    }

    // -------------------------------
    // Prompt user for ROM if missing
    // -------------------------------
    std::filesystem::path romPath = internalPath / "Starfox64.z64";
    if (!std::filesystem::exists(romPath)) {
        SDL_Log("No ROM found, prompting user...");

        nfdchar_t* outPath = nullptr;
        nfdresult_t result = NFD_OpenDialog("z64,rom", nullptr, &outPath);

        if (result == NFD_OKAY) {
            SDL_Log("User selected ROM: %s", outPath);
            std::filesystem::path selected(outPath);

            if (!copy_file(selected, romPath)) {
                show_error("Failed to copy ROM to internal storage.");
                free(outPath);
                return -1;
            }

            SDL_Log("ROM copied to: %s", romPath.string().c_str());
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
        SDL_Log("ROM already exists: %s", romPath.string().c_str());
    }

    // -------------------------------
    // Load ROM into memory
    // -------------------------------
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

    // -------------------------------
    // Initialize engine
    // -------------------------------
    if (!InitializeEngine(romData)) {
        show_error("Failed to initialize recompiled engine.");
        return -1;
    }

    // -------------------------------
    // Start main game loop
    // -------------------------------
    int ret = GameLoop();

    SDL_Quit();
    return ret;
}