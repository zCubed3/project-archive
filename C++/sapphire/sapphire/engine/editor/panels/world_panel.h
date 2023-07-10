#ifndef SAPPHIRE_WORLD_PANEL_H
#define SAPPHIRE_WORLD_PANEL_H

#if defined(IMGUI_SUPPORT)

#include <memory>
#include <engine/editor/panels/panel.h>

class World;
class Actor;

class WorldPanel : public Panel {
    REFLECT_CLASS(WorldPanel, Panel)

public:
    std::shared_ptr<World> world = nullptr;
    Actor* selected = nullptr;

    const char * get_title() override;

protected:
    void draw_actor_entry(Actor *p_actor);

    void draw_contents(Engine* p_engine) override;
};
#endif

#endif
