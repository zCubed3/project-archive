#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

// Opengl
#include <GL/glew.h>

// Engine
#include "assets/mesh.hpp"
#include "assets/shader.hpp"
#include "assets/texture.hpp"

#include "rendering/renderer.hpp"
#include "rendering/lighting.hpp"
#include "rendering/viewport.hpp"
#include "rendering/render_target.hpp"

#include "world/timing.hpp"
#include "world/world.hpp"
#include "world/actor.hpp"
#include "world/behavior.hpp"
#include "world/behaviors/camera.hpp"

#include "modularity/dynlib.hpp"
#include "modularity/gamemodule.hpp"

#include "data/console/console.hpp"
#include "data/console/cvar.hpp"
#include "data/console/cfunc.hpp"
#include "data/console/stdelems.hpp"

#include "data/engine_context.hpp"

#include "input/input_server.hpp"

#include "ui/imguicontext.hpp"

// Misc
#include <glm/mat4x4.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace Manta;
using namespace Manta::Rendering;
using namespace Manta::Data::Meshes;

typedef GameModule*(*module_init_fptr)();

// TODO: Replace old rendering architecture
#include <rendering/newrenderer.hpp>
#include <rendering/gl4/gl4renderer.hpp>
#include <rendering/vk/vkrenderer.hpp>

using namespace Manta::Rendering::OpenGL4;
using namespace Manta::Rendering::Vulkan;

// TODO: Differentiate between shipping and editor if we ever make an editor
int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);

    //NewRenderer* nrenderer = new GL4Renderer();
    NewRenderer* nrenderer = new VkRenderer();

    nrenderer->Initialize("Manta");

    SDL_Event sdl_event;

    bool keep_running = true;
    bool resized = false;
    bool first_run = true;

    auto engine = new EngineContext();
    engine->renderer2 = nrenderer;

    Mesh* test_mesh = Mesh::LoadFromFile("test.mmdl", engine);

    while (keep_running) {
        while (SDL_PollEvent(&sdl_event) != 0) {
            if (sdl_event.type == SDL_QUIT)
                keep_running = false;

            if (sdl_event.type == SDL_WINDOWEVENT) {
                if (sdl_event.window.event == SDL_WINDOWEVENT_RESIZED)
                    resized = true;
            }
        }

        if (first_run) {
            resized = true;
            first_run = false;
        }

        nrenderer->Present();
    }

    return 0;
    /*Old main loop
    SDL_Event sdl_event;
    bool keep_running = true;

    // TODO: Renderer modularity?
    auto renderer = new Rendering::Renderer();
    renderer->Initialize();

#ifdef WIN32
    auto dlib_game = DynLib::Open("./game.dll");
#else
    auto dlib_game = DynLib::Open("./lib/game.so");
#endif

    auto module_init = dlib_game->GetFunction<module_init_fptr>("module_init");
    GameModule* game_module = module_init();

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

    game_module->Initialize(engine);

    std::vector<std::string> safe_argv;

    for (int c = 1; c < argc; c++)
        safe_argv.emplace_back(argv[c]);

    Console::RegisterCommonCElems(engine->console);
    engine->console->DoCommandLine(safe_argv);

    safe_argv.clear();

    auto rt = new Rendering::RenderTarget(256, 256, Rendering::RenderTarget::DepthPrecision::Medium);
    rt->Create();

    auto viewport_transform = new Transform();
    viewport_transform->gen_view = true;

    auto viewport = new Rendering::Viewport();
    viewport->render_target = rt;

    engine->active_viewports.emplace_back(viewport);

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

        game_module->Update(engine);

        //
        // Drawing
        //
        if (resized)
            renderer->ClearScreen();

        // TODO: Dynamic lighting
        engine->lighting->UpdateBuffer();

        viewport_transform->position = glm::vec3(0, 0, 1);
        viewport_transform->euler = glm::vec3(engine->timing->sin_time.y * 20, 180 + engine->timing->cos_time.y * 20, 0);
        viewport_transform->UpdateMatrices();

        viewport->viewing_pos = viewport_transform->position;
        viewport->view = viewport_transform->view;
        viewport->UpdateViewport();

        for (auto target_viewport : engine->active_viewports) {
            engine->active_viewport = target_viewport;

            if (!target_viewport)
                continue;

            if (target_viewport->render_target) {
                renderer->SetRenderTarget(target_viewport->render_target);
            }

            renderer->SetViewportRect(target_viewport->rect, Renderer::ViewportSetFlags::SetViewport | Renderer::ViewportSetFlags::SetScissor);
            glClearColor(target_viewport->clear_color.x, target_viewport->clear_color.y, target_viewport->clear_color.z, 1.0f);

            int clear = 0;

            if (target_viewport->clear)
                clear |= GL_COLOR_BUFFER_BIT;

            if (target_viewport->clear_depth)
                clear |= GL_DEPTH_BUFFER_BIT;

            glClear(clear);

            // TODO: More generic draw loop
            game_module->Draw(engine);

            if (target_viewport->render_target) {
                renderer->SetRenderTarget(nullptr);
            }

            //mesh->DrawNow(test->transform.local_to_world, test->transform.world_to_local_t, shader);
        }

        // We have a special viewport change for the UI itself
        glViewport(0, 0, renderer->width, renderer->height);
        glScissor(0, 0,renderer->width, renderer->height);

        renderer->BeginImGui();

        game_module->DrawGUI(engine);

        ImGui::Begin("RT Test");

        ImGui::Image((void *) (intptr_t) rt->color_buffer->handle, ImVec2(rt->width, rt->height), ImVec2(0, 1), ImVec2(1, 0));

        ImGui::End();

        renderer->EndImGui();


        renderer->Present();
    }

    // Clean up
    delete renderer;
    delete dlib_game;

    return 0;
     */
}