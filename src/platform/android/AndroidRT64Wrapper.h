#pragma once

#include <cstdint>

struct SDL_Window;

namespace RT64 {
    struct RenderDevice;
    struct RenderContext;
}

namespace SF64RCA {

/**
 * AndroidRT64Wrapper
 *
 * This class owns the RT64 renderer lifecycle on Android.
 * It isolates RT64 from the rest of the engine so the submodule
 * remains untouched.
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

    RT64::RenderDevice* renderDevice = nullptr;
    RT64::RenderContext* renderContext = nullptr;

    int surfaceWidth = 0;
    int surfaceHeight = 0;
};

} // namespace SF64RCA