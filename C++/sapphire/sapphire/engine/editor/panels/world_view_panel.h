#ifndef SAPPHIRE_WORLD_VIEW_PANEL_H
#define SAPPHIRE_WORLD_VIEW_PANEL_H

#include <memory>

#include <engine/rendering/texture_render_target.h>

#if defined(IMGUI_SUPPORT)
#include <engine/editor/panels/panel.h>

class RenderServer;

class WorldViewPanel : public Panel {
    REFLECT_CLASS(WorldViewPanel, Panel)

    static int panel_count;

public:
    enum ViewMode {
        VIEW_MODE_COLOR,
        VIEW_MODE_DEPTH,

        VIEW_MODE_ENUM_MAX = VIEW_MODE_DEPTH
    };

    int id = 0;

    ViewMode view_mode = VIEW_MODE_COLOR;
    TextureRenderTarget* target = nullptr;
    std::shared_ptr<World> world = nullptr;

    bool enable_padding = false;
    int width;
    int height;
    bool automatic_size = true;

    glm::vec3 euler {};

    WorldViewPanel();
    ~WorldViewPanel() override;

    bool is_unique() override;
    int get_id() override;

    int get_imgui_flags() override;
    const char * get_title() override;
    void push_style_vars() override;
    void pop_style_vars() override;

    void draw_world(RenderServer *p_render_server);

    static const char* get_view_mode_name(ViewMode mode);

protected:
    void draw_contents(Engine* p_engine) override;
};
#endif

#endif
