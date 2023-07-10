#ifndef SAPPHIRE_TRANSFORM_H
#define SAPPHIRE_TRANSFORM_H

#include <glm.hpp>
#include <gtc/quaternion.hpp>

class Transform {
public:
    glm::mat4 trs {};
    glm::mat4 trs_inverse {};
    glm::mat4 trs_inverse_transpose {};

    glm::vec3 position = glm::vec3(0, 0, 0);
    glm::vec3 scale = glm::vec3(1, 1, 1);
    glm::quat quaternion = glm::identity<glm::quat>();

    glm::vec3 get_euler();
    void set_euler(glm::vec3 euler);

    void calculate_matrices();
};

#endif
