#ifndef MANTA_TEXTURE_HPP
#define MANTA_TEXTURE_HPP

#include <cstdint>

#include <vector>

namespace Manta {
    class Texture {
    public:
        enum class Dimension {
            TwoDimensional, Cubemap
        };

        enum class Format {
            RGB, RGBA, Depth, Depth32, Depth24, Depth16
        };

        enum class DataType {
            Byte, Float
        };

        Texture(int width, int height, Format format, DataType type);

        void CreateBuffer();
        void UpdateBuffer();

        Format format = Format::RGB;
        DataType type = DataType::Byte;
        int width = 128, height = 128;

        std::vector<unsigned char> byte_data;
        std::vector<float> float_data;

        uint32_t handle;
    };
}

#endif
