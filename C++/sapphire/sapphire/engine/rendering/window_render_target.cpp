#include "window_render_target.h"

#include <SDL.h>

#include <scene/world.h>

WindowRenderTarget::WindowRenderTarget(SDL_Window *p_window) {
    this->window = p_window;
}

Rect WindowRenderTarget::get_rect() {
    int width = 10;
    int height = 1;
    SDL_GetWindowSize(window, &width, &height);

    return {0, 0, width, height};
}

RenderTarget::TargetType WindowRenderTarget::get_type() {
    return TargetType::TARGET_TYPE_WINDOW;
}