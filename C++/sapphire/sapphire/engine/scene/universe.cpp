#include "universe.h"

#include <engine/engine.h>
#include <core/data/timing.h>

void Universe::add_world(const std::shared_ptr<World> &world) {
    loaded_worlds.push_back(world);
}

void Universe::tick(Engine *p_engine) {
    float delta = static_cast<float>(p_engine->timing->get_delta());

    for (std::shared_ptr<World>& world: loaded_worlds) {
        world->delta_time = delta;
        world->elapsed_time += delta;
    }
}
