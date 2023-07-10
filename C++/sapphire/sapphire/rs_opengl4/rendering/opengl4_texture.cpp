#include "opengl4_texture.h"

#include <glad/glad.h>

OpenGL4Texture::OpenGL4Texture(uint32_t handle, bool owns_handle) {
    this->handle = handle;
    this->owns_handle = owns_handle;
}

OpenGL4Texture::~OpenGL4Texture() {
    if (owns_handle && handle != -1) {
        glDeleteTextures(1, &handle);
    }
}

#if defined(IMGUI_SUPPORT)
void *OpenGL4Texture::get_imgui_handle() {
    return (void*)handle;
}
#endif

void OpenGL4Texture::load_bytes(unsigned char *bytes) {
    glActiveTexture(GL_TEXTURE0);

    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);

    int format = GL_UNSIGNED_BYTE;
    int data_format = GL_RGB;

    if (channels == 4) {
        data_format = GL_RGBA;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, data_format, width, height, 0, GL_RGBA, format, bytes);

    glGenerateMipmap(GL_TEXTURE_2D);
}

void OpenGL4Texture::load_faces(const std::vector<CubemapFace> &faces, unsigned char *shared_bytes, int shared_width, int shared_height) {
    glActiveTexture(GL_TEXTURE0);

    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, handle);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    if (shared_bytes != nullptr) {
        size_t length = shared_width * shared_height * 4;

        // Unlike Vulkan, we can't do rows and such
        // We have to copy this ourselves
        unsigned char* slice = new unsigned char[width * height * 4];
        size_t row_stride = width * channels;

        for (const CubemapFace& face: faces) {
            int face_index = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face.index;

            for (int y = 0; y < height; y++) {
                size_t y_cur = y + face.offset_y;

                for (int x = 0; x < width; x++) {
                    size_t x_cur = x + face.offset_x;

                    size_t src_offset = (y_cur * shared_width * channels) + (x_cur * channels);
                    size_t dst_offset = (y * width * channels) + (x * channels);

                    for (int c = 0; c < channels; c++) {
                        slice[dst_offset + c] = shared_bytes[src_offset + c];
                    }
                }
            }

            glTexImage2D(face_index, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, slice);
        }
    } else {
        for (const CubemapFace& face: faces) {
            int face_index = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face.index;
            glTexImage2D(face_index, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, face.unique_bytes);
        }
    }

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}
