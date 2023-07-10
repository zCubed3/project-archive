#include "gl4renderer.hpp"

#include <GL/glew.h>

#include <stdexcept>

namespace Manta::Rendering::OpenGL4 {
    void GL4Renderer::Initialize(const std::string& window_name) {
        sdl_window = SDL_CreateWindow(
                window_name.data(),
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                DEFAULT_WIDTH,
                DEFAULT_HEIGHT,
                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
        );

        // TODO: Fix depth buffer issues on Windows!

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        sdl_context = SDL_GL_CreateContext(sdl_window);

        // TODO: NewRenderer::SetVBlank()
        SDL_GL_SetSwapInterval(1);

        glewExperimental = true;
        if (glewInit() != 0) {
            throw std::runtime_error("GLEW failed to initialize!");
        }
    }

    void GL4Renderer::Present() {
        SDL_GL_SwapWindow(sdl_window);
    }
}