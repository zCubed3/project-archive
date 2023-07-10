#ifndef MANTA_ACTOR_HPP
#define MANTA_ACTOR_HPP

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "transform.hpp"

#include <string>
#include <vector>

namespace Manta {
    class Behavior;
    class Mesh;
    class Shader;
    class EngineContext;
    class World;

    class Actor {
    public:
        Transform transform = Transform();
        std::string name = "New Actor";
        bool enabled = true;

        std::vector<Mesh*> meshes;
        std::vector<Shader*> shaders;
        std::vector<Behavior*> behaviors;

        Actor(const std::string& name);

        void Update(World* world, EngineContext* engine);
        void Draw(World* world, EngineContext* engine);

        template<typename behavior_type>
        behavior_type* AddBehavior() {
            static_assert(std::is_base_of<Behavior, behavior_type>::value, "behavior_type be a derivative of Behavior!");

            auto b = new behavior_type();
            behaviors.emplace_back(b);
            return b;
        }
    };
}

#endif
