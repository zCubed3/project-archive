#include "world_panel.h"

#include <engine/engine.h>
#include <engine/scene/universe.h>

#if defined(IMGUI_SUPPORT)

#include <engine/scene/actor.h>
#include <engine/scene/world.h>

#include <imgui.h>

const char *WorldPanel::get_title() {
    return "World";
}

void WorldPanel::draw_actor_entry(Actor *p_actor) {
    ImGui::PushID(p_actor->id);

    int flags = 0;

    if (selected == p_actor) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    bool open = ImGui::TreeNodeEx(p_actor->name.c_str(), flags);

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        selected = p_actor;
    }

    ImGui::PopID();

    if (open) {
        for (Actor* child: p_actor->children) {
            draw_actor_entry(child);
        }

        ImGui::TreePop();
    }
}

void WorldPanel::draw_contents(Engine* p_engine) {
    std::string world_name = "None";

    if (world != nullptr) {
        world_name = world->name;
    }

    if (ImGui::BeginCombo("World", world_name.c_str())) {
        for (std::shared_ptr<World> select_world: p_engine->universe.loaded_worlds) {
            if (ImGui::Selectable(select_world->name.c_str(), world == select_world)) {
                world = select_world;
            }
        }

        ImGui::EndCombo();
    }

    if (world == nullptr) {
        ImGui::Text("No world!");
        return;
    }

    if (ImGui::Button("Add Actor")) {
        Actor *actor = new Actor();
        world->add_actor(actor);
    }

    ImGui::Spacing();

    ImGui::BeginChild("ActorTree", {0, 0}, true);
    for (Actor* actor: world->root_actors) {
        draw_actor_entry(actor);
    }
    ImGui::EndChild();
}

#endif