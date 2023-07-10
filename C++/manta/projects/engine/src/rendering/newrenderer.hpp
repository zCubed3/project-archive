#ifndef MANTA_NEWRENDERER_HPP
#define MANTA_NEWRENDERER_HPP

#include <SDL2/SDL.h>

#include <string>

namespace Manta {
    struct MeshBuffer;
    struct ShaderBuffer;

    class Mesh;
    class Shader;
}

namespace Manta::Rendering {
    class NewRenderer {
    public:
        // NOTE: Functions like SetDepthTestFunc and other renderer features are part of shaders now!
        // This is because Vulkan has pipelines, queues, and other AOT concepts, unlike OpenGL where everything drawn on demand
        // We have to specify ahead of time how we want a given command to be drawn via a pipeline!
        // OpenGL renderers will take the Shader's draw properties and set them
        // Vulkan will create a pipeline and cache it for that shader!

        //
        // Functions
        //

        // TODO: CreateWindow() and multiple windows?
        virtual void Initialize(const std::string& window_name) = 0;

        virtual void InitializeImGui() = 0;

        // TODO: Does vulkan need something different?
        virtual void Present() = 0;

        //
        // Renderer Assets
        //
        virtual ShaderBuffer* CreateShaderBuffer() = 0;
        virtual MeshBuffer* CreateMeshBuffer() = 0;

        //
        // Members
        //
        SDL_Window* sdl_window = nullptr;

    protected:
        const int DEFAULT_WIDTH = 1280, DEFAULT_HEIGHT = 720;
    };
}

#endif
