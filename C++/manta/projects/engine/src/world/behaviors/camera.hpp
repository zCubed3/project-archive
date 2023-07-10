#ifndef MANTA_CAMERA_HPP
#define MANTA_CAMERA_HPP

#include <world/behavior.hpp>

#include <rendering/viewport.hpp>

namespace Manta::Rendering {
    class RenderTarget;
}

namespace Manta {
    class CameraBehavior : public Behavior, public Rendering::Viewport {
    public:
        void OnDisable(World *world, Actor *owner, EngineContext* engine) override;
        void OnEnable(World *world, Actor *owner, EngineContext* engine) override;

        bool Update(World *world, Actor *owner, EngineContext *engine) override;

        std::string get_TypeId() override;
    };
}

#endif
