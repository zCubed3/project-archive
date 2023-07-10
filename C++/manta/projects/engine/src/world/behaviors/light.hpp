#ifndef MANTA_LIGHT_HPP
#define MANTA_LIGHT_HPP

#include <glm/vec3.hpp>

#include <world/behavior.hpp>

namespace Manta {
    class LightBehavior : public Behavior {
    public:
        enum LightType : int {
            Sun,
            Point,
            Spotlight
        };

        float intensity = 1.0f;
        float cone_angle = 60.0f, inner_perc = 50.0f;

        LightType light_type = LightType::Sun;
        glm::vec3 color = glm::vec3(1, 1, 1);

        void OnDisable(World *world, Actor *owner, EngineContext *engine) override;
        void OnEnable(World *world, Actor *owner, EngineContext *engine) override;

        std::string get_TypeId() override;
    };
}

#endif
