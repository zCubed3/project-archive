#include "render_target.h"

#include <engine/rendering/render_server.h>
#include <engine/rendering/render_target_data.h>
#include <engine/scene/world.h>

#include <gtc/matrix_transform.hpp>

RenderTarget::~RenderTarget() {
    delete rt_data;
    delete view_buffer;
}

void RenderTarget::begin_attach() {
    // The aspect is determined by the render target and not the camera!
    Rect rect = get_rect();
    float aspect = (float)rect.width / (float)rect.height;

    // Our view matrix is the inverse of the transform!
    const RenderServer* render_server = RenderServer::get_singleton();
    glm::vec3 correction = render_server->get_coordinate_correction();

    transform.calculate_matrices();

    if (view_matrix_mode == VIEW_MATRIX_MODE_AUTOMATIC) {
        view_data.camera_position = glm::vec4(transform.position, 1);
        view_data.view = transform.trs_inverse;
        view_data.projection = glm::perspective(glm::radians(fov), aspect, near_clip, far_clip);
        view_data.projection[1][1] *= correction.y;// Correction for Vulkan
        view_data.view_projection = view_data.projection * view_data.view;
    }

    if (world != nullptr) {
        view_data.time = glm::vec4(world->elapsed_time);
    }

    view_buffer->write(view_data);
}