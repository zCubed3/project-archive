#include "transform.h"

#include <cmath>

#include <gtc/matrix_transform.hpp>
#include <gtx/quaternion.hpp>

glm::vec3 Transform::get_euler() {
    return glm::eulerAngles(quaternion);
}

void Transform::set_euler(glm::vec3 euler) {
    quaternion = glm::quat(euler);
}

void Transform::calculate_matrices() {
    quaternion = glm::normalize(quaternion);

    glm::mat4 matrix = glm::identity<glm::mat4>();
    matrix = glm::translate(matrix, position);
    matrix *= glm::toMat4(quaternion);
    matrix = glm::scale(matrix, scale);

    this->trs = matrix;
    this->trs_inverse = glm::inverse(this->trs);
    this->trs_inverse_transpose = glm::transpose(this->trs_inverse);
}
