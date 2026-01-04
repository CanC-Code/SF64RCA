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
 * RendererBackend
 *
 * Allows selecting the graphics API backend.
 */
enum class RendererBackend {
    Vulkan,
    OpenGLES,
    Auto  // Default: Vulkan if available
};

/**
 * AndroidRT64Wrapper
 *
 * Bridges SDL window management and the RT64 renderer on Android.
 * Manages the RT64Context lifecycle and exposes basic render, resize, ROM load, and shutdown operations.
 */
class AndroidRT64Wrapper {
public:
    AndroidRT64Wrapper();
    ~AndroidRT64Wrapper();

    // Non-copyable
    AndroidRT64Wrapper(const AndroidRT64Wrapper&) = delete;
    AndroidRT64Wrapper& operator=(const AndroidRT64Wrapper&) = delete;

    /**
     * Initialize RT64 with an existing SDL window and optional backend.
     */
    bool initialize(SDL_Window* window, int width, int height, RendererBackend backend = RendererBackend::Auto);

    /**
     * Set rendering backend dynamically (future use).
     */
    void setBackend(RendererBackend backend);

    void resize(int width, int height);
    void renderFrame();
    bool loadRom(const uint8_t* data, size_t size);
    void shutdown();
    bool isInitialized() const;

private:
    bool initialized = false;
    SDL_Window* sdlWindow = nullptr;
    int surfaceWidth = 0;
    int surfaceHeight = 0;

    std::unique_ptr<uint8_t[]> rdram;
    std::unique_ptr<zelda64::renderer::RT64Context> renderContext;

    RendererBackend selectedBackend = RendererBackend::Auto;

    mutable std::mutex mutex; // Thread safety
};

} // namespace SF64RCA