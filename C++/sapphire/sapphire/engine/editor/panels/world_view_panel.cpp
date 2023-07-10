#include "world_view_panel.h"

#if defined(IMGUI_SUPPORT)
#include <engine/rendering/render_server.h>
#include <engine/rendering/texture.h>

#include <engine/scene/world.h>

#include <imgui.h>

#include <gtc/type_ptr.hpp>

int WorldViewPanel::panel_count = 0;

WorldViewPanel::WorldViewPanel() : Panel() {
    const RenderServer* rs_instance = RenderServer::get_singleton();

    target = new TextureRenderTarget(256, 256);
    width = 256;
    height = 256;

    rs_instance->populate_render_target_data(target);

    id = panel_count++;
}

WorldViewPanel::~WorldViewPanel() {
    if (panel_count > 0) {
        panel_count--;
    }

    delete target;
}

const char *WorldViewPanel::get_title() {
    return "World View";
}

int WorldViewPanel::get_imgui_flags() {
    return ImGuiWindowFlags_MenuBar;
}

bool WorldViewPanel::is_unique() {
    return true;
}

int WorldViewPanel::get_id() {
    return id;
}

void WorldViewPanel::draw_world(RenderServer *p_render_server) {
    if (world != nullptr) {
        target->resize(width, height);
        target->world = world;

        p_render_server->begin_target(target);

        world->draw();

        p_render_server->end_target(target);
    }
}

const char *WorldViewPanel::get_view_mode_name(ViewMode mode) {
    switch (mode) {
        default:
            return nullptr;

        case VIEW_MODE_COLOR:
            return "Color View";

        case VIEW_MODE_DEPTH:
            return "Depth View";
    }
}

void WorldViewPanel::push_style_vars() {
    if (!enable_padding && world != nullptr) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    }
}

void WorldViewPanel::pop_style_vars() {
    if (!enable_padding && world != nullptr) {
        ImGui::PopStyleVar();
    }
}

void WorldViewPanel::draw_contents(Engine* p_engine) {
    if (world == nullptr) {
        ImGui::Text("NO WORLD SELECTED!");
    } else {
        if (!enable_padding) {
            ImGui::PopStyleVar();
        }

        const RenderServer *rs_instance = RenderServer::get_singleton();
        float correction = -rs_instance->get_coordinate_correction().y;

        ImGui::BeginMenuBar();

        if (ImGui::BeginMenu("Transform")) {
            ImGui::DragFloat3("Position", glm::value_ptr(target->transform.position), 0.01F);
            ImGui::DragFloat4("Quaternion", glm::value_ptr(target->transform.quaternion), 0.01F);
            ImGui::DragFloat3("Scale", glm::value_ptr(target->transform.scale), 0.01F);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::DragFloat("Near Plane", &target->near_clip, 0.01F, 0.01F, 1000.0F);
            ImGui::DragFloat("Far Plane", &target->far_clip, 0.01F, 0.01F, 1000.0F);
            ImGui::DragFloat("FOV", &target->fov, 0.01F, 0.01F, 180.0F);
            ImGui::ColorEdit4("Clear Color", target->clear_color.backing);

            if (ImGui::BeginCombo("View Modes", get_view_mode_name(view_mode))) {
                for (int mode = VIEW_MODE_COLOR; mode <= VIEW_MODE_ENUM_MAX; mode++) {
                    if (ImGui::Selectable(get_view_mode_name((ViewMode) mode), view_mode == mode)) {
                        view_mode = (ViewMode) mode;
                    }
                }

                ImGui::EndCombo();
            }

            ImGui::Spacing();
            ImGui::Checkbox("Auto View Resolution", &automatic_size);

            ImGui::BeginDisabled(automatic_size);
            ImGui::DragInt("Width", &width, 1.0F, 1, 4096);
            ImGui::DragInt("Height", &height, 1.0F, 1, 4096);
            ImGui::EndDisabled();

            ImGui::EndMenu();
        }


        ImGui::EndMenuBar();

        ImVec2 content_size = ImGui::GetContentRegionAvail();

        if (content_size.x >= 1 && content_size.y >= 1) {
            Texture *texture = nullptr;

            switch (view_mode) {
                case VIEW_MODE_COLOR:
                    texture = target->get_color_texture();
                    break;

                case VIEW_MODE_DEPTH:
                    texture = target->get_depth_texture();
                    break;
            }

            if (texture != nullptr) {
                ImGui::Image(texture->get_imgui_handle(), content_size, {0, 0}, {1, correction});
            }

            ImGui::SetItemAllowOverlap();
            ImGui::SetCursorPos({0, 0});

            ImGuiIO &io = ImGui::GetIO();

            int flags = ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle;
            ImGui::InvisibleButton("RT_WINDOW", content_size, flags);

            glm::vec3 pan = glm::vec3(0, 0, 0);

            // TODO: User configurable speeds
            if (ImGui::IsItemHovered()) {
                pan += glm::vec3(0, 0, io.MouseWheel * io.DeltaTime * -5);
            }

            if (ImGui::IsItemActive()) {
                float horizontal = io.MouseDelta.x * io.DeltaTime;
                float vertical = io.MouseDelta.y * io.DeltaTime;

                if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
                    euler += glm::vec3(-vertical, -horizontal, 0);
                    target->transform.set_euler(euler);
                }

                if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
                    pan += glm::vec3(horizontal, -vertical, 0);
                }
            }

            target->transform.position += target->transform.quaternion * pan;

            if (automatic_size) {
                width = static_cast<int>(std::floor(content_size.x));
                height = static_cast<int>(std::floor(content_size.y));
            }
        }
    }
}
#endif