#include "object_buffer.h"

#include <engine/rendering/render_server.h>

ObjectBuffer::ObjectBuffer() {
    const RenderServer* rs_instance = RenderServer::get_singleton();

    if (rs_instance != nullptr) {
        buffer = rs_instance->create_graphics_buffer(sizeof(ObjectBufferData), GraphicsBuffer::UsageIntent::USAGE_INTENT_DEFAULT);
    }
}

ObjectBuffer::ObjectBuffer(GraphicsBuffer *p_buffer) {
    buffer = p_buffer;
}

ObjectBuffer::~ObjectBuffer() {
    delete buffer;
}

void ObjectBuffer::write(ObjectBufferData& data) {
    buffer->write(&data, sizeof(ObjectBufferData));
}