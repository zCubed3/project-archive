#ifndef SAPPHIRE_CONSOLE_PANEL_H
#define SAPPHIRE_CONSOLE_PANEL_H

#if defined(IMGUI_SUPPORT)
#include <vector>

#include <engine/console/console.h>
#include <engine/editor/panels/panel.h>

class ConsolePanel : public Panel {
    REFLECT_CLASS(ConsolePanel, Panel)

public:
    const char * get_title() override;

protected:
    std::string buffer;
    bool ignore_severities[3];

    void draw_contents(Engine* p_engine) override;
};
#endif

#endif
