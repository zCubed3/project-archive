#include "opengl4_graphics_buffer.h"

#include <glad/glad.h>

void OpenGL4GraphicsBuffer::write(void *data, size_t size) {
    if (buffer_handle != -1) {
        glBindBuffer(GL_UNIFORM_BUFFER, buffer_handle);
        glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW);
    }
}
