#ifndef MANTA_IMGUICONTEXT_HPP
#define MANTA_IMGUICONTEXT_HPP

#include <imgui.h>

#include <SDL2/SDL.h>

namespace Manta {
    class ImGuiContext {
    public:
        ImGuiContext(SDL_Window* window, SDL_GLContext context);

        void Process(SDL_Event* event);

        ImGuiStyle *style;
    };
}

#endif
