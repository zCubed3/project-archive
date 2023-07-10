#ifndef SAPPHIRE_VIEW_BUFFER_H
#define SAPPHIRE_VIEW_BUFFER_H

#include <glm.hpp>

#include <engine/rendering/buffers/graphics_buffer.h>

struct ViewBufferData {
    // Camera data
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 view_projection;
    glm::vec4 camera_position;

    // View-world local data
    glm::vec4 time;
};

class ViewBuffer {
public:
    GraphicsBuffer *buffer = nullptr;

    ViewBuffer(GraphicsBuffer *p_buffer);
    ~ViewBuffer();

    void write(ViewBufferData data);
};

#endif
