#include <SDL.h>
#include <SDL_main.h>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem>
#include <cstdio>
#include "nfd.h"

// Forward declarations
int GameLoop(); // Your existing game loop
bool InitializeEngine(const std::vector<uint8_t>& romData); // Engine init (patches, overlays, mods, shaders)

// Copy file safely
static bool copy_file(const std::filesystem::path& src, const std::filesystem::path& dst) {
    std::ifstream in(src, std::ios::binary);
    if (!in) return false;
    std::ofstream out(dst, std::ios::binary);
    if (!out) return false;
    out << in.rdbuf();
    return true;
}

// Show error via SDL dialog
static void show_error(const char* msg) {
    if (SDL_WasInit(SDL_INIT_VIDEO)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Starfox64 Error", msg, nullptr);
    }
    SDL_Log("ERROR: %s", msg);
}

extern "C"
int StarfoxMain(int argc, char** argv) {
    // Ensure SDL is initialized
    if (SDL_Init(0) < 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return -1;
    }

    // Determine internal storage path
    std::string internalDir(SDL_AndroidGetInternalStoragePath());
    if (internalDir.empty()) {
        show_error("Cannot access internal storage.");
        return -1;
    }
    std::filesystem::path romPath = std::filesystem::path(internalDir) / "Starfox64.z64";

    // First-launch: prompt for ROM if missing
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

    // Initialize engine (patches, overlays, mods, shaders)
    if (!InitializeEngine(romData)) {
        show_error("Failed to initialize recompiled engine.");
        return -1;
    }

    // Start main game loop
    int ret = GameLoop();

    SDL_Quit();
    return ret;
}