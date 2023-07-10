#ifndef SAPPHIRE_PANEL_H
#define SAPPHIRE_PANEL_H

#if defined(IMGUI_SUPPORT)
#include <engine/typing/class_registry.h>

class Engine;

class Panel {
    REFLECT_BASE_CLASS(Panel);

public:
    bool open = true;

    virtual ~Panel() = default;

    virtual bool can_close();
    virtual bool is_unique();
    virtual int get_id();

    virtual int get_imgui_flags();
    virtual const char* get_title() = 0;
    virtual void push_style_vars();
    virtual void pop_style_vars();

    virtual void draw_panel(Engine *p_engine);

protected:
    virtual void draw_contents(Engine *p_engine) = 0;
};
#endif

#endif
