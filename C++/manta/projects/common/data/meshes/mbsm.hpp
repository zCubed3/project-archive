#ifndef MANTA_COMMON_SMF_HPP
#define MANTA_COMMON_SMF_HPP

#include <cstdint>
#include <vector>
#include <string>

#include "../ident.hpp"

namespace Manta::Data::Meshes {
    // BSM, "Binary Static Mesh", a binary mesh format similar to OBJ!
    // One trade off is a bit of normal precision since we bake it as atan2(), asin() coords instead of a direction!
    class MantaBSM {
    public:
        static const uint32_t BSM_IDENT;

        struct BSMHeader {
            uint32_t ident; // "BSM#"
            uint32_t indice_count;
            uint32_t vertex_count;
            uint16_t name_len;
        };

        // Name follows after the header!
        // Indice block follows after name!

        struct BSMVertex {
            float position[3];
            float normal[2]; // Yaw Pitch compressed normals
            float uv[2];
            float tangent[3]; // Yaw Pitch compressed tangent
        };

        std::vector<BSMVertex> vertices;
        std::vector<uint32_t> indices;
        std::string name;

        static MantaBSM* LoadFromStream(std::istream& stream);
        static MantaBSM* LoadFromFile(const std::string &path);

        void WriteToFile(const std::string& path);
    };
}

#endif
