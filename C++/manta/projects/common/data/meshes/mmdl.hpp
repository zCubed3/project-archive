#ifndef MANTA_COMMON_MMDL_HPP
#define MANTA_COMMON_MMDL_HPP

#include "../ident.hpp"

#include <cstdint>
#include <vector>
#include <string>

namespace Manta::Data::Meshes {
    // Prototype of MantaMDL format!
    // V1 and future versions might change a lot!
    class MantaMDL {
    public:
        static const uint32_t MMDL_IDENT;

        enum ChannelType : uint8_t {
            NONE, // Special type, used by the writer mainly!
            SCALAR,
            VEC2,
            VEC3,
            VEC4,

            UINT,

            BONE
        };

        enum class ChannelHint : uint8_t {
            NONE, // Nothing special in here!

            // Common data
            VERTEX,
            NORMAL,
            TANGENT,
            UV0,
            INDEXER, // AKA Indices

            // Extra texture channels
            UV1,
            UV2,

            // Compressed data
            SMALL_NORMAL,
            SMALL_TANGENT,

            // Bone data
            SKELETON
        };

        struct Header {
            uint32_t ident;
            uint8_t descriptor_count;
            uint16_t name_len;
        };

        struct ChannelDescriptor {
            uint32_t read_len; // This value is read back and setting it manually does nothing!
            ChannelType type;
            ChannelHint hint;
        };

        struct Channel {
            ChannelDescriptor descriptor;
            std::vector<void*> data;
        };

        // Name is read, then channels are read
        // Channels are layered in series!

        struct Vec2 {
            float x, y;
        };

        struct Vec3 {
            float x, y, z;
        };

        struct Vec4 {
            float x, y, z, w;
        };

        struct Bone {
            Vec3 origin;
            Vec3 euler;
            Vec3 scale;
            uint32_t id;
            uint32_t connection; // If zero, this bone isn't parented!
        };

        ~MantaMDL();

        std::string name;
        std::vector<Channel*> channels;

        static MantaMDL* LoadFromStream(std::istream& stream);

        // Helpers for setting tags
        Channel* PushChannel(ChannelType type, ChannelHint hint);

        void SetChannelType(uint8_t idx, ChannelType type);
        void SetChannelHint(uint8_t idx, ChannelHint hint);
        void SetChannelProps(uint8_t idx, ChannelType type, ChannelHint hint);

        // Clears the data contained in a channel
        void ClearChannel(uint8_t idx);

    protected:
        static void* CloneData(void* data, size_t size);

    public:
        // Push helper
        void PushRawData(uint8_t idx, void* data);

        template<typename tdata>
        void PushData(uint8_t idx, tdata data) {
            PushRawData(idx, CloneData(&data, sizeof(tdata)));
        }

        void WriteToFile(const std::string& path);
    };
}

#endif
