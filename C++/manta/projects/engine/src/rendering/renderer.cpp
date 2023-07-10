#include "renderer.hpp"

#include <GL/glew.h>

#include <stdexcept>

#include <assets/texture.hpp>

#include <world/world.hpp>
#include <world/actor.hpp>

#include "viewport.hpp"
#include "render_target.hpp"

#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_opengl3.h>

#include "lighting.hpp"

namespace Manta::Rendering {
    void Renderer::Initialize() {
        sdl_context = nullptr;
        sdl_window = SDL_CreateWindow(
                "Manta",
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                1280,
                720,
                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
        );

        // TODO: Fix depth buffer issues on Windows!

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        // TODO: MSAA config
        //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,2);

        sdl_context = SDL_GL_CreateContext(sdl_window);
        SDL_GL_SetSwapInterval(1);

        glewExperimental = true;
        if (glewInit() != 0) {
            throw std::runtime_error("GLEW failed to initialize!");
        }

        glEnable(GL_DEPTH_TEST);
        SetDepthTest(DepthTestFunc::Less);

        glEnable(GL_CULL_FACE);
        SetCullMode(CullMode::Back);

        glEnable(GL_SCISSOR_TEST);
    }

    void Renderer::Update() {
        SDL_GetWindowSize(sdl_window, &width, &height);
    }

    void Renderer::DrawWorld(World *world, EngineContext* engine) {
        if (world == nullptr)
            throw std::runtime_error("world was nullptr!");

        for (const auto& actor : world->actors) {
            if (actor == nullptr)
                continue;

            actor->Draw(world, engine);
        }
    }

    void Renderer::BeginImGui() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(sdl_window);
        ImGui::NewFrame();
    }

    void Renderer::EndImGui() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void Renderer::ClearScreen() {
        glViewport(0, 0, width, height);
        glScissor(0, 0, width, height);
        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::SetViewportRect(ViewportRect rect, uint32_t options) {
        if (options & ViewportSetFlags::SetViewport)
            glViewport(rect.x, rect.y, rect.width, rect.height);

        if (options & ViewportSetFlags::SetScissor)
            glScissor(rect.x, rect.y, rect.width, rect.height);
    }

    void Renderer::SetRenderTarget(RenderTarget *target) {
        if (target == nullptr) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, target->fbo);

        if (target->color_buffer)
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target->color_buffer->handle, 0);

        if (target->depth_buffer)
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target->depth_buffer->handle, 0);
    }

    void Renderer::Present() {
        SDL_GL_SwapWindow(sdl_window);
    }

    void Renderer::SetCullMode(Renderer::CullMode mode) {
        culling_mode = mode;

        switch (mode) {
            default:
                glCullFace(GL_NONE);
                break;

            case CullMode::Front:
                glCullFace(GL_FRONT);
                break;

            case CullMode::Back:
                glCullFace(GL_BACK);
                break;
        }
    }

    void Renderer::SetDepthTest(DepthTestFunc func) {
        depth_testing = func;

        switch (func) {
            default:
                glDepthFunc(GL_ALWAYS);
                break;

            case DepthTestFunc::Less:
                glDepthFunc(GL_LESS);
                break;

            case DepthTestFunc::Greater:
                glDepthFunc(GL_GREATER);
                break;
        }
    }
}