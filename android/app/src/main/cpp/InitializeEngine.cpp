#include <vector>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <SDL.h>
#include "nfd.h"

// Forward declarations for your existing modules
bool LoadPatchesFromROM(const std::vector<uint8_t>& romData);
bool LoadNRMs(const std::filesystem::path& modDir);
bool CompileShaders(const std::filesystem::path& shaderDir);

bool InitializeEngine(const std::vector<uint8_t>& romData) {
    // 1. Generate patches dynamically from ROM
    if (!LoadPatchesFromROM(romData)) {
        SDL_Log("Failed to load patches from ROM");
        return false;
    }

    // 2. Load all NRM mods from assets folder
    std::filesystem::path modDir(SDL_AndroidGetInternalStoragePath());
    modDir /= "mods";
    if (!LoadNRMs(modDir)) {
        SDL_Log("Failed to load NRM mods");
        return false;
    }

    // 3. Compile shaders dynamically from assets folder
    std::filesystem::path shaderDir(SDL_AndroidGetInternalStoragePath());
    shaderDir /= "shaders";
    if (!CompileShaders(shaderDir)) {
        SDL_Log("Failed to compile shaders");
        return false;
    }

    SDL_Log("Engine initialized successfully");
    return true;
}