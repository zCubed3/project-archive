#include "panel.h"

#if defined(IMGUI_SUPPORT)

#include <string>

#include <random>
#include <imgui.h>

bool Panel::can_close() {
    return true;
}

bool Panel::is_unique() {
    return false;
}

int Panel::get_id() {
    return 0;
}

int Panel::get_imgui_flags() {
    return 0;
}

void Panel::push_style_vars() {

}

void Panel::pop_style_vars() {

}

void Panel::draw_panel(Engine* p_engine) {
    bool *p_open = can_close() ? &open : nullptr;

    if (open) {
        int flags = get_imgui_flags();

        std::string combo_id = get_title();
        if (is_unique()) {
            combo_id += "###";
            combo_id += std::to_string(get_id());
        }

        push_style_vars();

        if (ImGui::Begin(combo_id.c_str(), p_open, flags)) {
            draw_contents(p_engine);
        } else {
            pop_style_vars();
        }

        ImGui::End();
    }
}

#endif