#ifndef SAPPHIRE_UNIVERSE_H
#define SAPPHIRE_UNIVERSE_H

#include <vector>
#include <engine/scene/world.h>

class Engine;

// The universe stores the list of currently loaded worlds
// Multiple worlds can be loaded at once but only 1 Universe can exist at once
class Universe {
public:
    std::vector<std::shared_ptr<World>> loaded_worlds;

    void add_world(const std::shared_ptr<World>& world);

    void tick(Engine *p_engine);
};


#endif
