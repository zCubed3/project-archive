#include <iostream>

#include <data/engine_context.hpp>

#include <assets/texture.hpp>
#include <assets/mesh.hpp>
#include <assets/shader.hpp>

#include <world/timing.hpp>
#include <world/world.hpp>
#include <world/actor.hpp>

#include <world/behaviors/camera.hpp>

#include <rendering/render_target.hpp>
#include <rendering/viewport.hpp>
#include <rendering/renderer.hpp>
#include <rendering/lighting.hpp>

#include <data/console/console.hpp>

#include <ui/imguicontext.hpp>

#include <input/input_server.hpp>

#include <GL/glew.h>

#include <glm/gtc/quaternion.hpp>

using namespace Manta;

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);

    SDL_Event sdl_event;
    bool keep_running = true;

    // TODO: Renderer modularity?
    auto renderer = new Rendering::Renderer();
    renderer->Initialize();

    auto imgui = new Manta::ImGuiContext(renderer->sdl_window, renderer->sdl_context);

    auto input = new Input::InputServer();

    auto engine = new EngineContext();

    engine->renderer = renderer;
    engine->timing = new Timing();
    engine->console = new Console::Console();
    engine->imgui = imgui;
    engine->input = input;
    engine->lighting = new Rendering::Lighting();
    engine->lighting->CreateBuffer();

    Shader::CreateEngineShaders(engine);

    // The empty world is used for the blank scene!
    auto empty_world = new World();

    // By default we have one giant scene camera pointed at 0, 0, 0
    auto scene_camera_actor = new Actor("scene_camera");
    auto scene_camera = scene_camera_actor->AddBehavior<CameraBehavior>();

    empty_world->AddActor(scene_camera_actor);

    engine->worlds.emplace_back(empty_world);

    // TODO: Replace me with an actual mesh viewer!
    Mesh* mmdl_proto = Mesh::LoadFromFile("test.mmdl");

    std::vector<World*> mesh_view_worlds;

    auto mesh_proto_world = new World();

    auto mesh_proto_actor = new Actor("dummy_actor");
    mesh_proto_actor->meshes.emplace_back(mmdl_proto);
    mesh_proto_world->AddActor(mesh_proto_actor);

    auto mesh_proto_actor2 = new Actor("dummy_actor2");
    mesh_proto_actor2->meshes.emplace_back(mmdl_proto);
    mesh_proto_world->AddActor(mesh_proto_actor2);

    auto mesh_proto_camera_actor = new Actor("viewer_camera");
    mesh_proto_camera_actor->transform.position = glm::vec3(0, 0, 1);
    mesh_proto_camera_actor->transform.euler = glm::vec3(0, 180, 0);

    auto mesh_proto_camera = mesh_proto_camera_actor->AddBehavior<CameraBehavior>();
    mesh_proto_world->AddActor(mesh_proto_camera_actor);

    auto rt = new Rendering::RenderTarget(256, 256, Rendering::RenderTarget::DepthPrecision::High);
    rt->Create();
    mesh_proto_camera->render_target = rt;

    engine->worlds.emplace_back(mesh_proto_world);

    mesh_view_worlds.emplace_back(mesh_proto_world);
    bool was_dragging_left = false, was_dragging_right = false;
    glm::vec3 drag_offset = glm::vec3(0, 0, 0), globe_pos = glm::vec3(0, 0, 1);

    bool first_run = true;
    while (keep_running) {
        // Event polling
        bool resized = false;

        while (SDL_PollEvent(&sdl_event) != 0) {
            if (sdl_event.type == SDL_QUIT)
                keep_running = false;

            if (sdl_event.type == SDL_WINDOWEVENT) {
                if (sdl_event.window.event == SDL_WINDOWEVENT_RESIZED)
                    resized = true;
            }

            input->ProcessEvent(&sdl_event);
            imgui->Process(&sdl_event);
        }

        if (first_run) {
            resized = true;
            first_run = false;
        }

        renderer->Update();
        engine->timing->UpdateTime();

        input->UpdateBinds();

        // Empty world update
        scene_camera->rect.width = renderer->width;
        scene_camera->rect.height = renderer->height;

        empty_world->Update(engine);

        for (auto world : mesh_view_worlds)
            world->Update(engine);

        //
        // Draw the empty scene
        //
        engine->active_viewport = scene_camera;
        renderer->SetViewportRect(scene_camera->rect);

        glClearColor(scene_camera->clear_color.x, scene_camera->clear_color.y,
                     scene_camera->clear_color.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderer->DrawWorld(empty_world, engine);

        //
        // Draw the mesh viewer world
        //
        engine->active_viewport = mesh_proto_camera;
        renderer->SetViewportRect(mesh_proto_camera->rect);

        renderer->SetRenderTarget(mesh_proto_camera->render_target);

        glClearColor(mesh_proto_camera->clear_color.x, mesh_proto_camera->clear_color.y,
                     mesh_proto_camera->clear_color.z, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderer->DrawWorld(mesh_proto_world, engine);

        renderer->SetRenderTarget(nullptr);


        //
        // Packer UI
        //
        glViewport(0, 0, renderer->width, renderer->height);
        glScissor(0, 0,renderer->width, renderer->height);

        renderer->BeginImGui();

        ImGui::ShowDemoWindow();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(renderer->width / 3, renderer->height));

        ImGui::Begin("Content Folders", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        ImGui::End();

        // TODO: Make me into an actual mesh viewer!
        ImGui::SetNextWindowSize(ImVec2(0, 0));

        ImGui::Begin("Mesh Viewer Proto", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

        auto cursor = ImGui::GetCursorPos();
        ImGui::InvisibleButton("mesh_viewer_box", ImVec2(256, 256));

        if (ImGui::IsItemHovered())
        {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
                was_dragging_left = true;

            if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
                was_dragging_right = true;

            mesh_proto_camera->fov -= input->mouse_scroll_y * 1.0f;

            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }

        if (was_dragging_left && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
            was_dragging_left = false;

        if (was_dragging_right && !ImGui::IsMouseDown(ImGuiMouseButton_Right))
            was_dragging_right = false;

        if (was_dragging_left || was_dragging_right) {
            glm::vec3 up = mesh_proto_camera_actor->transform.local_to_world * glm::vec4(0, 1, 0, 0);
            glm::vec3 right = mesh_proto_camera_actor->transform.local_to_world * glm::vec4(1, 0, 0, 0);

            if (was_dragging_left) {
                globe_pos += up * input->mouse_delta_y * 0.01f;
                globe_pos += right * input->mouse_delta_x * 0.01f;
                globe_pos = glm::normalize(globe_pos);

                auto look_quat = glm::quatLookAt(globe_pos, glm::vec3(0, 1, 0));
                mesh_proto_camera_actor->transform.euler = glm::degrees(glm::eulerAngles(look_quat));
            }

            if (was_dragging_right) {
                drag_offset += up * input->mouse_delta_y * 0.01f;
                drag_offset += right * input->mouse_delta_x * 0.01f;
            }

            mesh_proto_actor2->transform.position = drag_offset;
            mesh_proto_camera_actor->transform.position = drag_offset + globe_pos;
        }

        ImGui::SetCursorPos(cursor);

        auto image_size = ImVec2(mesh_proto_camera->rect.width, mesh_proto_camera->rect.height);
        ImGui::Image((void *) (intptr_t) mesh_proto_camera->render_target->color_buffer->handle, image_size, ImVec2(0, 1), ImVec2(1, 0));

        ImGui::End();

        renderer->EndImGui();


        renderer->Present();

        input->Reset();
    }

    return 0;
}