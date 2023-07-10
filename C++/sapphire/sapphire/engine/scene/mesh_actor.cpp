#include "mesh_actor.h"

#include <engine/scene/world.h>

#include <engine/assets/material_asset.h>
#include <engine/assets/mesh_asset.h>
#include <engine/rendering/buffers/object_buffer.h>
#include <engine/rendering/objects/mesh_draw_object.h>
#include <engine/rendering/render_server.h>
#include <engine/rendering/render_target.h>

MeshActor::MeshActor() {
    draw_object = new MeshDrawObject();
}

MeshActor::~MeshActor() {
    Actor::~Actor();

    delete draw_object;
}

void MeshActor::draw(World *p_world) {
    Actor::draw(p_world);

    RenderServer* rs_instance = RenderServer::get_singleton();

    transform.calculate_matrices();

    ObjectBufferData data {};
    data.model = transform.trs;
    data.model_inverse = transform.trs_inverse;
    data.model_inverse_transpose = transform.trs_inverse_transpose;
    data.model_view_projection = rs_instance->get_current_target()->view_data.view_projection * transform.trs;

    draw_object->update_buffer(data);

    if (mesh_asset) {
        draw_object->mesh_buffer = mesh_asset->buffer;
    } else {
        draw_object->mesh_buffer = nullptr;
    }

    if (material_asset) {
        draw_object->material = material_asset->material;
    } else {
        draw_object->material = nullptr;
    }

    if (entered_world) {
        p_world->enqueue_mesh_draw_object(draw_object);
        entered_world = false;
    }
}

void MeshActor::on_enter_world(World *p_world) {
    // We wait a frame before enqueuing our objects so the proper initialization can occur
    // TODO: Make initialization immediate?
    entered_world = true;
}