#include "viewport.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "render_target.hpp"

namespace Manta::Rendering {
    void Viewport::UpdateViewport() {
        float aspect = (float)rect.width / (float)rect.height;
        perspective = glm::perspective(glm::radians(fov), aspect, z_near, z_far);

        eye = perspective * view;
    }
}