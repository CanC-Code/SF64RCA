#pragma once

namespace SF64RCA {

enum class RendererBackend {
    Vulkan,
    OpenGLES
};

RendererBackend SelectBestRenderer();

}