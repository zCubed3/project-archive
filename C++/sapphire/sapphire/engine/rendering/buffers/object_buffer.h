#ifndef SAPPHIRE_OBJECT_BUFFER_H
#define SAPPHIRE_OBJECT_BUFFER_H

#include <glm.hpp>

#include <engine/rendering/buffers/graphics_buffer.h>

struct ObjectBufferData {
    glm::mat4 model_view_projection;
    glm::mat4 model;
    glm::mat4 model_inverse;
    glm::mat4 model_inverse_transpose;
};

class ObjectBuffer {
public:
    GraphicsBuffer *buffer = nullptr;

    ObjectBuffer();
    ObjectBuffer(GraphicsBuffer *p_buffer);
    ~ObjectBuffer();

    void write(ObjectBufferData& data);
};

#endif
