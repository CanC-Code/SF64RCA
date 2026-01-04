#pragma once

#include <cstdint>
#include <memory>
#include <mutex>

struct SDL_Window;

namespace zelda64::renderer {
    class RT64Context;
}

namespace SF64RCA {

/**
 * AndroidRT64Wrapper
 *
 * Bridges SDL window management and the RT64 renderer on Android.
 * Manages the RT64Context lifecycle and exposes basic render, resize, and shutdown operations.
 */
class AndroidRT64Wrapper {
public:
    AndroidRT64Wrapper();
    ~AndroidRT64Wrapper();

    // Non-copyable
    AndroidRT64Wrapper(const AndroidRT64Wrapper&) = delete;
    AndroidRT64Wrapper& operator=(const AndroidRT64Wrapper&) = delete;

    /**
     * Initialize RT64 with an existing SDL window.
     * Returns false on failure.
     */
    bool initialize(SDL_Window* window, int width, int height);

    /**
     * Resize the rendering surface.
     */
    void resize(int width, int height);

    /**
     * Render a single frame.
     */
    void renderFrame();

    /**
     * Shut down RT64 cleanly.
     */
    void shutdown();

    /**
     * Whether RT64 is fully initialized.
     */
    bool isInitialized() const;

private:
    bool initialized = false;
    SDL_Window* sdlWindow = nullptr;

    int surfaceWidth = 0;
    int surfaceHeight = 0;

    std::unique_ptr<uint8_t[]> rdram;
    std::unique_ptr<zelda64::renderer::RT64Context> renderContext;

    mutable std::mutex mutex; // Thread safety
};

} // namespace SF64RCA