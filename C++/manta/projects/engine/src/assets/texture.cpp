#include "texture.hpp"

#include <GL/glew.h>

// TODO: Don't depend on OpenGL entirely?
namespace Manta {
    Texture::Texture(int width, int height, Format format, DataType type) {
        this->width = width;
        this->height = height;
        this->format = format;
        this->type = type;
    }

    void Texture::CreateBuffer() {
        glGenTextures(1, &handle);
        UpdateBuffer();
    }

    // TODO: Cubemaps and 3D textures!
    void Texture::UpdateBuffer() {
        glBindTexture(GL_TEXTURE_2D, handle);

        void* data = nullptr;
        uint32_t fmt = GL_UNSIGNED_BYTE;

        switch (type) {
            case DataType::Byte:
                data = byte_data.data();
                break;

            case DataType::Float:
                data = float_data.data();
                fmt = GL_FLOAT;
                break;

            default:
                break;
        }

        int32_t gl_format = GL_DEPTH_COMPONENT;
        bool is_depth = true;
        switch (format) {
            case Format::Depth:
                gl_format = GL_DEPTH_COMPONENT;
                break;

            case Format::Depth16:
                gl_format = GL_DEPTH_COMPONENT16;
                break;

            case Format::Depth24:
                gl_format = GL_DEPTH_COMPONENT24;
                break;

            case Format::Depth32:
                gl_format = GL_DEPTH_COMPONENT32;
                break;

            case Format::RGB:
                is_depth = false;
                gl_format = GL_RGB;
                break;

            case Format::RGBA:
                is_depth = false;
                gl_format = GL_RGBA;
                break;

            default:
                break;
        }

        // TODO: Proper texture loading
        if (is_depth)
            glTexImage2D(GL_TEXTURE_2D, 0, gl_format, width, height, 0, GL_DEPTH_COMPONENT, fmt, nullptr);
        else
            glTexImage2D(GL_TEXTURE_2D, 0, gl_format, width, height, 0, gl_format, fmt, data);

        // TODO: Proper texture filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
}