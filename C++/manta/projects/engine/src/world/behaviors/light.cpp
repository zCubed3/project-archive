#include "light.hpp"

#include <data/engine_context.hpp>

#include <rendering/lighting.hpp>

#include <algorithm>

namespace Manta {
    void LightBehavior::OnDisable(World *world, Actor *owner, EngineContext* engine) {
        auto iter = std::find(engine->lighting->lights.begin(), engine->lighting->lights.end(), this);

        if (iter != engine->lighting->lights.end())
            engine->lighting->lights.erase(iter);
    }

    void LightBehavior::OnEnable(World *world, Actor *owner, EngineContext* engine) {
        engine->lighting->lights.emplace_back(this);
    }

    std::string LightBehavior::get_TypeId() { return "light"; }
}