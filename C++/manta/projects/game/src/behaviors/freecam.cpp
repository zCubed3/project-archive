#include "freecam.hpp"

#include <world/timing.hpp>
#include <world/actor.hpp>

#include <data/engine_context.hpp>

#include <input/axis.hpp>
#include <input/input_server.hpp>

#include <iostream>

namespace Manta::Game {
    bool FreecamBehavior::Update(World* world, Actor *owner, EngineContext *engine) {
        if (!Behavior::Update(world, owner, engine))
            return false;

        glm::vec3 forward = owner->transform.local_to_world * glm::vec4(0, 0, 1, 0);
        glm::vec3 right = owner->transform.local_to_world * glm::vec4(1, 0, 0, 0);


        auto horizontal_axis = engine->input->bound_axes["horizontal"];
        owner->transform.position -= right * horizontal_axis->value * engine->timing->delta_time;

        auto vertical_axis = engine->input->bound_axes["vertical"];
        owner->transform.position += forward * vertical_axis->value * engine->timing->delta_time;

        owner->transform.euler.y -= engine->input->mouse_delta_x * 0.1f;
        owner->transform.euler.x += engine->input->mouse_delta_y * 0.1f;

        return true;
    }

    std::string FreecamBehavior::get_TypeId() { return "freecamera"; }
}