#ifndef SAPPHIRE_WINDOW_TARGET_H
#define SAPPHIRE_WINDOW_TARGET_H

#include "render_target.h"

#if defined(IMGUI_SUPPORT)
#include <imgui.h>
#endif

typedef struct SDL_Window SDL_Window;

class WindowRenderTarget : public RenderTarget {
public:
    SDL_Window *window;

#if defined(IMGUI_SUPPORT)
    ImGuiContext* imgui_context;
#endif

    WindowRenderTarget() = delete;
    WindowRenderTarget(const WindowRenderTarget &) = delete;

    WindowRenderTarget(SDL_Window *p_window);

    Rect get_rect() override;

    TargetType get_type() override;
};

#endif
