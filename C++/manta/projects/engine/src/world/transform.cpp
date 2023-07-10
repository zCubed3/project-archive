#include "transform.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Manta {
    Transform::Transform(glm::vec3 position, glm::vec3 euler, glm::vec3 scale) {
        this->position = position;
        this->euler = euler;
        this->scale = scale;
    }

    void Transform::UpdateMatrices() {
        local_to_world = glm::translate(glm::mat4(1.0f), position);
        local_to_world *= glm::toMat4(glm::quat(glm::radians(euler)));
        local_to_world = glm::scale(local_to_world, scale);

        if (!only_local) {
            world_to_local = glm::inverse(local_to_world);
            world_to_local_t = glm::transpose(world_to_local);
        }

        if (gen_view) {
            glm::vec3 up = local_to_world * glm::vec4(0, 1, 0, 0);
            glm::vec3 forward = local_to_world * glm::vec4(0, 0, 1, 0);
            view = glm::lookAt(position, position + forward, up);
        }
    }
}