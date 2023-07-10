#ifndef SAPPHIRE_RENDERER_PANEL_H
#define SAPPHIRE_RENDERER_PANEL_H

#if defined(IMGUI_SUPPORT)
#include <vector>
#include <engine/editor/panels/panel.h>

class RendererPanel : public Panel {
    REFLECT_CLASS(RendererPanel, Panel)

public:
    const char * get_title() override;

protected:
    std::vector<float> previous_deltas;
    int max_deltas = 100;

    void draw_contents(Engine* p_engine) override;
};
#endif

#endif
