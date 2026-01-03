#pragma once

#include <cstdint>
#include <vector>
#include <memory>

// Forward declare RT64 types to avoid including heavy headers in all files
struct RT64RenderContext;
struct RT64FrameSettings;

class AndroidRT64Wrapper {
public:
    AndroidRT64Wrapper();
    ~AndroidRT64Wrapper();

    // Initialize RT64 with platform-specific context
    bool initialize(void* windowHandle, int width, int height);

    // Shutdown and cleanup
    void shutdown();

    // Render a single frame using current game data
    void renderFrame(const uint8_t* frameBuffer, int width, int height);

    // Resize viewport if window changes
    void resize(int width, int height);

    // Access underlying RT64 context if needed
    RT64RenderContext* getContext();

private:
    RT64RenderContext* context;
};