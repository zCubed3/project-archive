#include "mesh_draw_object.h"

#include <engine/rendering/material.h>
#include <engine/rendering/buffers/mesh_buffer.h>

MeshDrawObject::MeshDrawObject() {
    object_buffer = new ObjectBuffer();
}

MeshDrawObject::~MeshDrawObject() {
    delete object_buffer;
}

void MeshDrawObject::update_buffer(ObjectBufferData& data) {
    object_buffer->write(data);
}

void MeshDrawObject::draw() {
    mesh_buffer->draw(object_buffer, material);
}