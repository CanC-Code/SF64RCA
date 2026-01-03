#include "AndroidRT64Wrapper.h"

// Include RT64 headers from submodule
#include "../../../lib/rt64/include/rt64.h"
#include "../../../lib/rt64/include/rt64_render_context.h"

#include <iostream>

AndroidRT64Wrapper::AndroidRT64Wrapper()
    : context(nullptr)
{
}

AndroidRT64Wrapper::~AndroidRT64Wrapper()
{
    shutdown();
}

bool AndroidRT64Wrapper::initialize(void* windowHandle, int width, int height)
{
    if (context) {
        std::cerr << "[RT64] Already initialized\n";
        return false;
    }

    context = RT64CreateRenderContext();
    if (!context) {
        std::cerr << "[RT64] Failed to create context\n";
        return false;
    }

    RT64InitSettings settings{};
    settings.windowHandle = windowHandle;
    settings.width = width;
    settings.height = height;
    settings.enableUpscaling = true;
    settings.enableInterpolation = true;

    if (!RT64Initialize(context, &settings)) {
        std::cerr << "[RT64] Initialization failed\n";
        context = nullptr;
        return false;
    }

    std::cout << "[RT64] Initialized successfully\n";
    return true;
}

void AndroidRT64Wrapper::shutdown()
{
    if (context) {
        RT64Shutdown(context);
        context = nullptr;
        std::cout << "[RT64] Shutdown complete\n";
    }
}

void AndroidRT64Wrapper::renderFrame(const uint8_t* frameBuffer, int width, int height)
{
    if (!context) return;

    RT64FrameSettings frameSettings{};
    frameSettings.frameBuffer = frameBuffer;
    frameSettings.width = width;
    frameSettings.height = height;

    RT64RenderFrame(context, &frameSettings);
}

void AndroidRT64Wrapper::resize(int width, int height)
{
    if (!context) return;

    RT64ResizeViewport(context, width, height);
}

RT64RenderContext* AndroidRT64Wrapper::getContext()
{
    return context;
}