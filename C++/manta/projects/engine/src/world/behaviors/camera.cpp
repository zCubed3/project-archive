#include "camera.hpp"

#include <algorithm>

#include <world/actor.hpp>
#include <world/world.hpp>

#include <rendering/render_target.hpp>

#include <data/engine_context.hpp>

namespace Manta {
    void CameraBehavior::OnDisable(World *world, Actor *owner, EngineContext* engine) {
        auto iter = std::find(engine->active_viewports.begin(), engine->active_viewports.end(), this);

        if (iter != engine->active_viewports.end())
            engine->active_viewports.erase(iter);
    }

    void CameraBehavior::OnEnable(World *world, Actor *owner, EngineContext* engine) {
        engine->active_viewports.emplace_back(this);
    }

    bool CameraBehavior::Update(World *world, Actor *owner, EngineContext *engine) {
        if (!Behavior::Update(world, owner, engine))
            return false;

        if (render_target) {
            rect.width = render_target->width;
            rect.height = render_target->height;
        }

        owner->transform.gen_view = true;

        view = owner->transform.view;
        viewing_pos = owner->transform.position;

        UpdateViewport();

        return true;
    }

    std::string CameraBehavior::get_TypeId() { return "camera"; }
}