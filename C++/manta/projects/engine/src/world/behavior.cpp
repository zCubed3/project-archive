#include "behavior.hpp"

#include "actor.hpp"

namespace Manta {
    void Behavior::Start(World *world, Actor *owner, EngineContext* engine) {
        if (enabled)
            OnEnable(world, owner, engine);
        else
            OnDisable(world, owner, engine);

        is_new = false;
    }

    bool Behavior::Update(World *world, Actor* owner, EngineContext* engine) {
        if (last_enabled != enabled) {
            if (enabled)
                OnEnable(world, owner, engine);
            else
                OnDisable(world, owner, engine);
        }

        this->owner = owner;

        last_enabled = enabled;

        if (!enabled || !owner->enabled)
            return false;

        return true;
    }

    void Behavior::OnDisable(World *world, Actor *owner, EngineContext* engine) {}
    void Behavior::OnEnable(World *world, Actor *owner, EngineContext* engine) {}

    bool Behavior::IsNew() {
        return is_new;
    }
}