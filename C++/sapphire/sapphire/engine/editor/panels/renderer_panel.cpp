#include "renderer_panel.h"

#if defined(IMGUI_SUPPORT)

#include <imgui.h>

#include <engine/rendering/render_server.h>
#include <core/data/timing.h>

const char *RendererPanel::get_title() {
    return "Renderer";
}

void RendererPanel::draw_contents(Engine* p_engine) {
    const RenderServer* rs_instance = RenderServer::get_singleton();
    const Timing* timing = Timing::get_singleton();

    if (ImGui::CollapsingHeader("API")) {
        ImGui::Text("Rendering API: %s", rs_instance->get_name());
    }

    if (ImGui::CollapsingHeader("Stats")) {
        while (previous_deltas.size() < max_deltas) {
            previous_deltas.push_back(0.0F);
        }

        double delta = timing->get_delta();

        ImGui::Text("Raw Delta: %f", delta);
        ImGui::PlotLines("Previous Deltas", previous_deltas.data(), 100);

        ImGui::DragInt("Max History", &max_deltas, 1.0F, 1);

        previous_deltas.push_back(static_cast<float>(delta));

        while (previous_deltas.size() > max_deltas) {
            previous_deltas.erase(previous_deltas.begin());
        }
    }
}
#endif