#ifndef MANTA_RENDERER_HPP
#define MANTA_RENDERER_HPP

#include <SDL2/SDL.h>

#include "viewport.hpp"

namespace Manta {
    class World;
    class EngineContext;
}

namespace Manta::Rendering {
    class Lighting;
    class RenderTarget;

    class Renderer {
    public:
        Viewport* active_viewport = nullptr;

        void Initialize();

        void Update();

        void ClearScreen();

        void DrawWorld(World* world, EngineContext* engine);

        void BeginImGui();
        void EndImGui();

        //
        // Higher level draw calls
        //
        enum ViewportSetFlags : uint32_t {
            SetViewport = 1,
            SetScissor = 2
        };

        void SetViewportRect(ViewportRect rect, uint32_t options = ViewportSetFlags::SetViewport);

        // Pass nullptr to draw to the default framebuffer!
        void SetRenderTarget(RenderTarget* target);

        // Drawing properties
        enum class CullMode {
            Off, Back, Front
        };

        CullMode culling_mode = CullMode::Off;
        void SetCullMode(CullMode mode);

        enum class DepthTestFunc {
            Off, Less, Greater
        };

        DepthTestFunc depth_testing = DepthTestFunc::Off;
        void SetDepthTest(DepthTestFunc func);

        void Present();

        int width = 512, height = 512;

        SDL_GLContext sdl_context = nullptr;
        SDL_Window* sdl_window = nullptr;
    };
}

#endif
