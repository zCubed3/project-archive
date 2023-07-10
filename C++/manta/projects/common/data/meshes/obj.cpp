#include "obj.hpp"

#include <vector>
#include <memory>

namespace Manta::Data::Meshes {
    static void weld_obj(WavefrontOBJ* obj,
                       const std::vector<WavefrontOBJ::OBJIndice> triangles,
                       const std::vector<WavefrontOBJ::OBJVector3> positions,
                       const std::vector<WavefrontOBJ::OBJVector3> normals,
                       const std::vector<WavefrontOBJ::OBJVector2> uvs,
                       const std::vector<WavefrontOBJ::OBJVector4> tangents) {

        obj->weld_vertices.clear();
        obj->weld_indices.clear();

        bool tan_too = !tangents.empty();

        for (const auto &tri: triangles) {
            WavefrontOBJ::OBJVertex vert{};

            vert.position[0] = positions[tri.v].x;
            vert.position[1] = positions[tri.v].y;
            vert.position[2] = positions[tri.v].z;

            vert.uv[0] = uvs[tri.u].x;
            vert.uv[1] = uvs[tri.u].y;

            vert.normal[0] = normals[tri.n].x;
            vert.normal[1] = normals[tri.n].y;
            vert.normal[2] = normals[tri.n].z;

            if (tan_too) {
                vert.tangent[0] = tangents[tri.v].x;
                vert.tangent[1] = tangents[tri.v].y;
                vert.tangent[2] = tangents[tri.v].z;
                vert.tangent[3] = tangents[tri.v].w;
            }

            bool similarVert = false;
            for (int v = 0; v < obj->weld_vertices.size(); v++)
                if (obj->weld_vertices[v].Compare(vert, tan_too)) {
                    obj->weld_indices.emplace_back(v);
                    similarVert = true;
                    break;
                }

            if (!similarVert) {
                obj->weld_indices.emplace_back(obj->weld_vertices.size());
                obj->weld_vertices.emplace_back(vert);
            }
        }
    }

    bool WavefrontOBJ::OBJVertex::Compare(const OBJVertex &rhs, bool tan_too) {
        bool s = position[0] == rhs.position[0] &&
                position[1] == rhs.position[1] &&
                position[2] == rhs.position[2] &&
                normal[0] == rhs.normal[0] &&
                normal[1] == rhs.normal[1] &&
                normal[2] == rhs.normal[2] &&
                uv[0] == rhs.uv[0] &&
                uv[1] == rhs.uv[1];

        if (tan_too)
            s = s &&
                tangent[0] == rhs.tangent[0] &&
                tangent[1] == rhs.tangent[1] &&
                tangent[2] == rhs.tangent[2] &&
                tangent[3] == rhs.tangent[3];

        return s;
    }

    WavefrontOBJ* WavefrontOBJ::LoadFromStream(std::istream& stream, bool weld_tris) {
        auto obj = new WavefrontOBJ();

        obj->welded = weld_tris;

        std::vector<OBJVector3> positions;
        std::vector<OBJVector3> normals;
        std::vector<OBJVector2> uvs;
        std::vector<OBJIndice> triangles;

        std::string line;
        while (std::getline(stream, line)) {
            // First two characters are a data id
            std::string id = line.substr(0, 2);

            if (id[0] == '#')
                continue;

            std::string contents = line.substr(2);

            if (id[0] == 'o')
                obj->name = contents;

            if (id[0] == 'v') {
                if (id[1] == 't') {
                    OBJVector2 v2 {};
                    sscanf(contents.c_str(), "%f %f", &v2.x, &v2.y);
                    uvs.emplace_back(v2);
                    continue;
                }

                OBJVector3 v3data {};
                sscanf(contents.c_str(), "%f %f %f", &v3data.x, &v3data.y, &v3data.z);

                if (id[1] == 'n')
                    normals.emplace_back(v3data);
                else
                    positions.emplace_back(v3data);
            }

            // F = face, we either iterate 3 times to make a tri, or once to make it a vert, depends on the obj
            // Ugly method of loading faces but idc enough to fix it
            if (id[0] == 'f') {
                std::string face = contents;

                size_t pos;
                while (true) {
                    OBJIndice tri {};
                    pos = face.find(' ');

                    sscanf(face.c_str(), "%i/%i/%i",
                           &tri.v, &tri.u, &tri.n
                    );

                    face.erase(0, pos + 1);

                    tri.v -= 1;
                    tri.u -= 1;
                    tri.n -= 1;

                    triangles.emplace_back(tri);

                    if (pos == std::string::npos)
                        break;
                }
            }
        }

        if (weld_tris) {
            weld_obj(obj, triangles, positions, normals, uvs, {});
        } else {
            for (const auto& p : positions)
                obj->unweld_positions.push_back({p.x, p.y, p.z});

            for (const auto& n : normals)
                obj->unweld_normals.push_back({n.x, n.y, n.z});

            for (const auto& u : uvs)
                obj->unweld_uvs.push_back({u.x, u.y});

            for (const auto& i : triangles)
                obj->unweld_indices.push_back({i.v, i.u, i.n});
        }

        return obj;
    }

    void WavefrontOBJ::WeldVertices() {
        weld_obj(this, unweld_indices, unweld_positions, unweld_normals, unweld_uvs, unweld_tangents);
    }
}