#include "opengl4_render_server.h"

#include <SDL.h>
#include <glad/glad.h>

#include <engine/assets/asset_loader.h>
#include <engine/assets/mesh_asset.h>
#include <engine/rendering/render_target.h>
#include <engine/rendering/texture_render_target.h>
#include <engine/rendering/window_render_target.h>
#include <engine/rendering/objects/mesh_draw_object.h>
#include <engine/scene/world.h>

#include <rs_opengl4/rendering/opengl4_mesh_buffer.h>
#include <rs_opengl4/rendering/opengl4_graphics_buffer.h>
#include <rs_opengl4/rendering/opengl4_shader.h>
#include <rs_opengl4/rendering/opengl4_texture.h>
#include <rs_opengl4/rendering/opengl4_material.h>
#include <rs_opengl4/rendering/opengl4_render_target_data.h>

#if defined(IMGUI_SUPPORT)
#include <imgui.h>
#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_opengl3.h>
#endif

void OpenGL4RenderServer::register_rs_asset_loaders() {

}

const char* OpenGL4RenderServer::get_name() const {
    return "OpenGL 4.6";// TODO: Other versions?
}

std::string OpenGL4RenderServer::get_error() const {
    switch (error) {
        case ERR_SINGLETON_CLASH:
            return "Singleton not nullptr!";

        case ERR_WINDOW_NULLPTR:
            return "window was nullptr!";

        case ERR_CONTEXT_NULLPTR:
            return "gl_context was nullptr!";
    }

    return "";
}

uint32_t OpenGL4RenderServer::get_sdl_window_flags() const {
    return SDL_WINDOW_OPENGL;
}

// TODO: Abstract window class?
// Should the window be abstracted or should viewports altogether be abstracted?

/*
void GLAPIENTRY gl_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam ) {
    fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}
*/

bool OpenGL4RenderServer::initialize(SDL_Window *p_window) {
    if (singleton != nullptr) {
        error = ERR_SINGLETON_CLASH;
        return false;
    }

    if (p_window == nullptr) {
        error = ERR_WINDOW_NULLPTR;
        return false;
    }

#ifdef DEBUG
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    gl_context = SDL_GL_CreateContext(p_window);

    if (gl_context == nullptr) {
        error = ERR_CONTEXT_NULLPTR;
        return false;
    }

    if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
        return false;
    }

    singleton = this;
    window = p_window;

    // TODO: VSync option
    SDL_GL_SetSwapInterval(1);

    OpenGL4Shader::create_error_shader();

    // TODO: Move these to materials?
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

#ifdef DEBUG
    /*
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(gl_message_callback, nullptr);
    */
#endif

    return true;
}

bool OpenGL4RenderServer::present(SDL_Window *p_window) {
    if (p_window == nullptr) {
        error = ERR_WINDOW_NULLPTR;
        return false;
    }

    return true;
}

bool OpenGL4RenderServer::begin_frame() {
    return true;
}

bool OpenGL4RenderServer::end_frame() {
    return true;
}

bool OpenGL4RenderServer::begin_target(RenderTarget *p_target) {
    if (p_target == nullptr) {
        error = ERR_TARGET_NULLPTR;
        return false;
    }

    if (p_target->get_type() == RenderTarget::TARGET_TYPE_WINDOW) {
        WindowRenderTarget *sdl_target = reinterpret_cast<WindowRenderTarget *>(p_target);

        SDL_GL_MakeCurrent(sdl_target->window, gl_context);
    }

    if (p_target->get_type() == RenderTarget::TARGET_TYPE_TEXTURE) {
        TextureRenderTarget *texture_target = reinterpret_cast<TextureRenderTarget *>(p_target);
        OpenGL4RenderTargetData* data = reinterpret_cast<OpenGL4RenderTargetData*>(texture_target->rt_data);

        glBindFramebuffer(GL_FRAMEBUFFER, data->framebuffer_handle);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, data->depth_texture_handle, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data->texture_handle, 0);

       // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, data->depthbuffer_handle);

        uint32_t attachments[] = {GL_DEPTH_ATTACHMENT, GL_COLOR_ATTACHMENT0};
        glDrawBuffers(2, attachments);
    }

    // TODO: Not set this per frame?
    // TODO: If and when Vulkan is added, make this conformant to Vulkan and not OpenGL!
    Rect rect = p_target->get_rect();
    glViewport(rect.x, rect.y, rect.width, rect.height);

    if (p_target->clear_flags != 0) {
        uint32_t clear_flags = 0;

        if (p_target->clear_flags & RenderTarget::ClearFlags::CLEAR_FLAG_DEPTH) {
            clear_flags |= GL_DEPTH_BUFFER_BIT;
        }

        if (p_target->clear_flags & RenderTarget::ClearFlags::CLEAR_FLAG_COLOR) {
            clear_flags |= GL_COLOR_BUFFER_BIT;
            glClearColor(p_target->clear_color[0], p_target->clear_color[1], p_target->clear_color[2], p_target->clear_color[3]);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    p_target->begin_attach();

    // TODO: Let's find a better way to do this :P
    current_target = p_target;

    return true;
}

bool OpenGL4RenderServer::end_target(RenderTarget *p_target) {
    if (p_target == nullptr) {
        error = ERR_TARGET_NULLPTR;
        return false;
    }

    // TODO: Allow rendering without a world?
    if (p_target->world != nullptr) {
        for (const auto& shader_iter: p_target->world->draw_tree) {
            OpenGL4ShaderPass *shader_pass = nullptr;

            if (shader_iter.first != nullptr) {
                shader_pass = reinterpret_cast<OpenGL4ShaderPass *>(shader_iter.first->get_pass("Lit"));
            }

            bool fallback = shader_pass == nullptr;
            if (fallback) {
                shader_pass = reinterpret_cast<OpenGL4ShaderPass *>(OpenGL4Shader::error_shader->passes[0]);
            }

            shader_pass->bind();

            for (const auto& material_iter: shader_iter.second) {
                if (!fallback) {
                    material_iter.first->bind_material(shader_pass);
                }

                // Our object data should've already been updated by this moment
                // We just have to update the binding
                for (MeshDrawObject *object: material_iter.second) {
                    shader_pass->setup_object(object);
                    object->draw();
                }
            }
        }
    }

    if (p_target->get_type() == RenderTarget::TARGET_TYPE_WINDOW) {
        WindowRenderTarget *sdl_target = reinterpret_cast<WindowRenderTarget *>(p_target);

        SDL_GL_SwapWindow(sdl_target->window);
    }

    if (p_target->get_type() == RenderTarget::TARGET_TYPE_TEXTURE) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    current_target = nullptr;

    return true;
}

// TODO: Use the usage parameter
GraphicsBuffer *OpenGL4RenderServer::create_graphics_buffer(size_t size, GraphicsBuffer::UsageIntent usage) const {
    uint32_t buffer_handle;
    glGenBuffers(1, &buffer_handle);
    glBindBuffer(GL_UNIFORM_BUFFER, buffer_handle);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    OpenGL4GraphicsBuffer *buffer = new OpenGL4GraphicsBuffer();
    buffer->buffer_handle = buffer_handle;

    return buffer;
}

Shader *OpenGL4RenderServer::create_shader() const {
    return new OpenGL4Shader();
}

Texture *OpenGL4RenderServer::create_texture() const {
    return new OpenGL4Texture();
}

Material *OpenGL4RenderServer::create_material() const {
    return new OpenGL4Material();
}

void OpenGL4RenderServer::populate_mesh_buffer(MeshAsset *p_mesh_asset) const {
    if (p_mesh_asset != nullptr) {
        p_mesh_asset->buffer = std::shared_ptr<MeshBuffer>(new OpenGL4MeshBuffer(p_mesh_asset));
    }
}

void OpenGL4RenderServer::populate_render_target_data(RenderTarget *p_render_target) const {
    if (p_render_target != nullptr) {
        RenderServer::populate_render_target_data(p_render_target);

        if (p_render_target->get_type() == RenderTarget::TARGET_TYPE_TEXTURE) {
            TextureRenderTarget *image_target = reinterpret_cast<TextureRenderTarget*>(p_render_target);

            // TODO: Abstract opengl framebuffers?
            OpenGL4RenderTargetData* data = new OpenGL4RenderTargetData();
            glGenFramebuffers(1, &data->framebuffer_handle);

            Rect rect = image_target->get_rect();

            data->resize(rect.width, rect.height, p_render_target);
            image_target->rt_data = data;
        }
    }
}

#if defined(IMGUI_SUPPORT)
void OpenGL4RenderServer::initialize_imgui(WindowRenderTarget *p_target) {
    ImGuiContext* old_context = ImGui::GetCurrentContext();

    RenderServer::initialize_imgui(p_target);

    ImGui_ImplSDL2_InitForOpenGL(p_target->window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");

    ImGui::SetCurrentContext(old_context);
}

bool OpenGL4RenderServer::begin_imgui(WindowRenderTarget *p_target) {
    ImGui::SetCurrentContext(p_target->imgui_context);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(p_target->window);
    ImGui::NewFrame();

    return true;
}

bool OpenGL4RenderServer::end_imgui(WindowRenderTarget *p_target) {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return true;
}
#endif