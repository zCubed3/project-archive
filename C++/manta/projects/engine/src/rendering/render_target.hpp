#ifndef MANTA_RENDER_TARGET_HPP
#define MANTA_RENDER_TARGET_HPP

#include <cstdint>

namespace Manta {
    class Texture;
}

namespace Manta::Rendering {
    class RenderTarget {
    public:
        enum class DepthPrecision {
            None = 0, Low = 16, Medium = 24, High = 32
        };

        RenderTarget(int width, int height, DepthPrecision depth_precision = DepthPrecision::None);

        void Create();

        int width, height;
        DepthPrecision depth_precision;

        Texture *color_buffer, *depth_buffer;

        uint32_t fbo = 0, depth_rbo = 0;
    };
}

#endif
