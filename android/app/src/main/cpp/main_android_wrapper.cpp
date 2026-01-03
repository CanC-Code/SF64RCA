#include <SDL.h>
#include <string>
#include <fstream>
#include <filesystem>
#include <cstdio>
#include "nfd.h"

// Forward declaration of your real desktop main()
int main(int argc, char** argv);

// Android-safe entry point called from SDL_main
extern "C"
int StarfoxMain(int argc, char** argv) {
    // Determine app's internal storage path
    std::string romDir(SDL_AndroidGetInternalStoragePath()); // SDL helper for Android
    if (romDir.empty()) {
        SDL_Log("ERROR: Cannot get internal storage path");
        return -1;
    }

    std::filesystem::path romPath = std::filesystem::path(romDir) / "Starfox64.z64";

    // Check if ROM already exists
    if (!std::filesystem::exists(romPath)) {
        SDL_Log("No Starfox64 ROM found, prompting user...");

        nfdchar_t *outPath = nullptr;
        nfdresult_t result = NFD_OpenDialog("z64,rom", nullptr, &outPath);

        if (result == NFD_OKAY) {
            SDL_Log("User selected ROM: %s", outPath);

            // Copy selected ROM to internal storage
            std::ifstream src(outPath, std::ios::binary);
            std::ofstream dst(romPath, std::ios::binary);

            if (!src || !dst) {
                SDL_Log("ERROR: Failed to copy ROM to internal storage");
                free(outPath);
                return -1;
            }

            dst << src.rdbuf();
            src.close();
            dst.close();

            SDL_Log("ROM copied to: %s", romPath.string().c_str());
            free(outPath);
        } else if (result == NFD_CANCEL) {
            SDL_Log("User cancelled ROM selection.");
            return -1;
        } else {
            SDL_Log("NFD error: %s", NFD_GetError());
            return -1;
        }
    } else {
        SDL_Log("ROM already exists: %s", romPath.string().c_str());
    }

    // Pass ROM path as argv[1] to your main
    char* argv_new[2];
    argv_new[0] = argv[0];
    argv_new[1] = const_cast<char*>(romPath.string().c_str());

    return main(2, argv_new);
}