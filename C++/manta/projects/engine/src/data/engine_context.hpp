#ifndef MANTA_ENGINE_CONTEXT_HPP
#define MANTA_ENGINE_CONTEXT_HPP

#include <vector>

// Stores dynamically allocated parts of the engine inside a single class that is passed into modules for usage
namespace Manta {
    class Timing;
    class Shader;
    class ImGuiContext;
    class World;

    namespace Console {
        class Console;
    }

    namespace Input {
        class InputServer;
    }

    namespace Rendering {
        class Lighting;
        class Renderer;
        class Viewport;
        class NewRenderer;
    }

    class EngineContext {
    public:
        //
        // Render data
        //
        std::vector<Rendering::Viewport*> active_viewports;
        Rendering::Viewport* active_viewport;
        Rendering::Lighting* lighting;
        Rendering::Renderer* renderer;
        Rendering::NewRenderer* renderer2;

        Timing* timing;

        Console::Console* console;
        Input::InputServer* input;

        ImGuiContext* imgui;

        std::vector<World*> worlds;

        // TODO: Move stupid data like this elsewhere
        Shader* error_shader;
    };
}

#endif
