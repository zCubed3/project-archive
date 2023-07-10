#ifndef MANTA_GL4RENDERER_HPP
#define MANTA_GL4RENDERER_HPP

#include <rendering/newrenderer.hpp>

namespace Manta::Rendering::OpenGL4 {
    // TODO: GLES or MultiGL renderer?
    // A renderer that supports OpenGL 4.0+ contexts
    // This is to target higher end desktops!
    // For mobile or lower end, target GL3 / GL3ES, or possibly Vulkan!
    class GL4Renderer : public NewRenderer {
    public:
        void Initialize(const std::string& window_name) override;

        void Present() override;

    protected:
        SDL_GLContext sdl_context = nullptr;
    };
}


#endif
