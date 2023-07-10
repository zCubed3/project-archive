#ifndef MANTA_COMMON_OBJ_HPP
#define MANTA_COMMON_OBJ_HPP

#include <iostream>
#include <string>
#include <vector>
#include <memory>

namespace Manta::Data::Meshes {
    class WavefrontOBJ {
    public:
        typedef float OBJ_FLOAT2[2];
        typedef float OBJ_FLOAT3[3];
        typedef float OBJ_FLOAT4[4];

        struct OBJVertex {
            OBJ_FLOAT3 position;
            OBJ_FLOAT3 normal;
            OBJ_FLOAT2 uv;
            OBJ_FLOAT4 tangent;

            bool Compare(const OBJVertex& rhs, bool tan_too);
        };

        struct OBJIndice {
            uint32_t v = 0;
            uint32_t u = 0;
            uint32_t n = 0;
        };

        struct OBJVector2 {
            float x, y;
        };

        struct OBJVector3 {
            float x, y, z;
        };

        struct OBJVector4 {
            float x, y, z, w;
        };

        std::string name = "Unknown";

        bool welded = false;

        std::vector<OBJVertex> weld_vertices = {};
        std::vector<uint32_t> weld_indices = {};

        std::vector<OBJVector3> unweld_positions = {};
        std::vector<OBJVector3> unweld_normals = {};
        std::vector<OBJVector4> unweld_tangents = {}; // Equal to normal indices + only matters when baking, is not populated by default!
        std::vector<OBJVector2> unweld_uvs = {};
        std::vector<OBJIndice> unweld_indices = {};

        static WavefrontOBJ* LoadFromStream(std::istream& stream, bool weld_tris = true);
        void WeldVertices();
    };
}

#endif
