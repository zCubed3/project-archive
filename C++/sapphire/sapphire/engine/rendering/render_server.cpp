#include "render_server.h"

#include <engine/assets/mesh_asset.h>
#include <engine/rendering/buffers/mesh_buffer.h>
#include <engine/rendering/buffers/object_buffer.h>
#include <engine/rendering/render_target.h>
#include <engine/rendering/window_render_target.h>
#include <engine/rendering/objects/mesh_draw_object.h>

#if defined(IMGUI_SUPPORT)
#include <imgui.h>
#endif

RenderServer *RenderServer::singleton = nullptr;

RenderServer::~RenderServer() {

}

RenderServer *RenderServer::get_singleton() {
    return singleton;
}

RenderTarget *RenderServer::get_current_target() const {
    return current_target;
}

glm::vec3 RenderServer::get_coordinate_correction() const {
    return {1, 1, 1};
}

void RenderServer::on_window_resized(SDL_Window *p_window) {

}

void RenderServer::populate_render_target_data(RenderTarget *p_render_target) const {
    if (p_render_target->view_buffer == nullptr) {
        GraphicsBuffer* buffer = create_graphics_buffer(sizeof(ViewBufferData), GraphicsBuffer::UsageIntent::USAGE_INTENT_DEFAULT);
        p_render_target->view_buffer = new ViewBuffer(buffer);
    }
}

#if defined(IMGUI_SUPPORT)
void RenderServer::initialize_imgui(WindowRenderTarget *p_target) {
    p_target->imgui_context = ImGui::CreateContext();
    ImGui::SetCurrentContext(p_target->imgui_context);

    //ImGuiStyle &style = ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    //ImGui::StyleColorsClassic(&style);
}

void RenderServer::release_imgui(WindowRenderTarget *p_target) {

}
#endif
