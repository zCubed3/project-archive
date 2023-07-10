#include "light.h"

#include <engine/rendering/render_server.h>
#include <engine/rendering/texture_render_target.h>
#include <engine/scene/world.h>

Light::Light() {
    const RenderServer* rs_instance = RenderServer::get_singleton();

    shadow = new TextureRenderTarget(2048, 2048, TextureRenderTarget::USAGE_INTENT_SHADOW);
    shadow->view_matrix_mode = RenderTarget::VIEW_MATRIX_MODE_MANUAL;

    rs_instance->populate_render_target_data(shadow);

    buffer = rs_instance->create_graphics_buffer(sizeof(LightShadowData), GraphicsBuffer::UsageIntent::USAGE_INTENT_DEFAULT);
}

Light::~Light() {
    delete shadow;
    delete buffer;
}

void Light::render_shadows(RenderServer *p_render_server, World *p_world) {
    // TODO: Other types of light
    // TODO: Better calculate the light matrices

    const RenderServer* rs_instance = RenderServer::get_singleton();
    float correction = rs_instance->get_coordinate_correction().y;

    shadow->view_data.view = glm::lookAt(sun_pos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    shadow->view_data.projection = glm::ortho(-5.0F, 5.0F, -5.0F, 5.0F, 0.01F, 100.0F);
    shadow->view_data.projection[1][1] *= correction;
    shadow->view_data.view_projection = shadow->view_data.projection * shadow->view_data.view;

    LightShadowData data {};
    data.light_matrix = shadow->view_data.view_projection;
    data.light_position = glm::vec4(sun_pos, 0.0F);

    buffer->write(&data, sizeof(LightShadowData));

    p_render_server->begin_target(shadow);

    p_world->draw();

    p_render_server->end_target(shadow);
}
