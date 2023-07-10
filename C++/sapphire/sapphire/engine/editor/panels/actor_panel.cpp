#include "actor_panel.h"

#if defined(IMGUI_SUPPORT)

#include <engine/scene/actor.h>

#include <imgui.h>

const char *ActorPanel::get_title() {
    return "Actor";
}

void ActorPanel::draw_contents(Engine* p_engine) {
    if (world != nullptr) {
        if (target != nullptr) {
            target->draw_imgui(world.get());
        } else {
            ImGui::Text("No actor selected!");
        }
    } else {
        ImGui::Text("NO WORLD ASSIGNED!");
    }
}

#endif