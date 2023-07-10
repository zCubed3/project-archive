#include "render_target.hpp"

#include <GL/glew.h>

#include <assets/texture.hpp>

namespace Manta::Rendering {
    RenderTarget::RenderTarget(int width, int height, DepthPrecision depth_precision) {
        this->width = width;
        this->height = height;
        this->depth_precision = depth_precision;
    }

    void RenderTarget::Create() {
        color_buffer = new Texture(width, height, Texture::Format::RGB, Texture::DataType::Byte);
        color_buffer->CreateBuffer();

        if (depth_precision != DepthPrecision::None) {
            auto depth_format = Texture::Format::Depth;

            switch (depth_precision) {
                case DepthPrecision::Low:
                    depth_format = Texture::Format::Depth16;
                    break;

                case DepthPrecision::Medium:
                    depth_format = Texture::Format::Depth24;
                    break;

                case DepthPrecision::High:
                    depth_format = Texture::Format::Depth32;
                    break;

                default:
                    break;
            }

            depth_buffer = new Texture(width, height, depth_format, Texture::DataType::Float);
            depth_buffer->CreateBuffer();
        }

        glGenFramebuffers(1, &fbo);
    }
}