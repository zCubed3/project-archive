#ifndef MANTA_TRANSFORM_HPP
#define MANTA_TRANSFORM_HPP

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace Manta {
    struct Transform {
        glm::vec3 position;
        glm::vec3 euler;
        glm::vec3 scale;

        glm::mat4 local_to_world;
        glm::mat4 world_to_local;
        glm::mat4 world_to_local_t;

        // Not set by default! Only set if gen_view is true!
        glm::mat4 view;

        bool only_local = false;
        bool gen_view = false;

        Transform(glm::vec3 position = glm::vec3(0, 0, 0), glm::vec3 euler = glm::vec3(0, 0, 0), glm::vec3 scale = glm::vec3(1, 1, 1));

        void UpdateMatrices();
    };
}

#endif
