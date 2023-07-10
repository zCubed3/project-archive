#ifndef SAPPHIRE_OPENGL4_GRAPHICS_BUFFER_H
#define SAPPHIRE_OPENGL4_GRAPHICS_BUFFER_H

#include <cstdint>
#include <engine/rendering/buffers/graphics_buffer.h>

class OpenGL4GraphicsBuffer : public GraphicsBuffer {
public:
    uint32_t buffer_handle = -1;

    void write(void *data, size_t size) override;
};

#endif
