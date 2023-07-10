#include "imguicontext.hpp"

#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_opengl3.h>

namespace Manta {
    // TODO: OpenGL independent?
    ImGuiContext::ImGuiContext(SDL_Window* window, SDL_GLContext context) {
        ImGui::CreateContext();

        ImGui_ImplSDL2_InitForOpenGL(window, context);
        ImGui_ImplOpenGL3_Init("#version 150");

        style = &ImGui::GetStyle();
        ImGui::StyleColorsDark(style);

        ImGuiIO &imguiIO = ImGui::GetIO();
        imguiIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        imguiIO.Fonts->AddFontFromFileTTF("content/engine/fonts/Roboto-Regular.ttf", 14);
    }

    void ImGuiContext::Process(SDL_Event *event) {
        ImGui_ImplSDL2_ProcessEvent(event);
    }
}