#include "obj_loader.h"

#include <fstream>

#include <engine/assets/static_mesh_asset.h>
#include <engine/rendering/render_server.h>

struct OBJTriangle {
    uint32_t v = 0;
    uint32_t t = 0;
    uint32_t n = 0;
};

void OBJLoader::load_placeholders() {
    StaticMeshAsset::create_primitives();
}

std::vector<std::string> OBJLoader::get_extensions() {
    return {"obj"};
}

std::shared_ptr<Asset> OBJLoader::load_from_path(const std::string &path, const std::string& extension) {
    std::ifstream file(path);

    if (file.is_open()) {
        std::vector<glm::vec3> unweld_positions;
        std::vector<glm::vec3> unweld_normals;
        std::vector<glm::vec2> unweld_tex_coords;
        std::vector<OBJTriangle> unweld_triangles;
        std::string name = "";

        std::string line;
        while (std::getline(file, line)) {
            // First two characters are commonly a data id
            std::string id = line.substr(0, 2);

            // Comment
            if (id[0] == '#') {
                continue;
            }

            std::string contents = line.substr(2);

            // TODO: Named meshes?
            /*
            if (id[0] == 'o') {
                obj->name = contents;
            }
            */

            // Vertex data
            if (id[0] == 'v') {

                // Texcoord
                if (id[1] == 't') {
                    // TODO: Replace sscanf?
                    glm::vec2 uv{};

                    sscanf(contents.c_str(), "%f %f", &uv.x, &uv.y);

                    // OBJ UVs are upside down
                    uv.y = 1.0F - uv.y;

                    unweld_tex_coords.emplace_back(uv);

                    continue;
                }

                glm::vec3 v3_data{};
                sscanf(contents.c_str(), "%f %f %f", &v3_data.x, &v3_data.y, &v3_data.z);

                if (id[1] == 'n')
                    unweld_normals.emplace_back(v3_data);
                else
                    unweld_positions.emplace_back(v3_data);
            }

            // F = face, we either iterate 3 times to make a tri, or once to make it a vert, depends on the obj
            // TODO: Make this less ugly?
            if (id[0] == 'f') {
                std::string face = contents;

                size_t pos;
                while (true) {
                    OBJTriangle tri{};
                    pos = face.find(' ');

                    sscanf(face.c_str(), "%i/%i/%i",
                           &tri.v, &tri.t, &tri.n);

                    face.erase(0, pos + 1);

                    tri.v -= 1;
                    tri.t -= 1;
                    tri.n -= 1;

                    unweld_triangles.emplace_back(tri);

                    if (pos == std::string::npos)
                        break;
                }
            }

            if (id[0] == 'o') {
                name = contents;
            }
        }

        // We then need to weld the triangles
        std::vector<glm::vec3> weld_positions;
        std::vector<glm::vec3> weld_normals;
        std::vector<glm::vec2> weld_tex_coords;
        std::vector<uint32_t> weld_triangles;
        uint32_t welded = 0;

//#define NO_WELD

#ifndef NO_WELD
        for (size_t t = 0; t < unweld_triangles.size(); t++) {
            OBJTriangle triangle = unweld_triangles[t];

            glm::vec3 position = unweld_positions[triangle.v];
            glm::vec2 tex_coord = unweld_tex_coords[triangle.t];
            glm::vec3 normal = unweld_normals[triangle.n];

            // Check that no existing vertex matches this one (this becomes exponentially slower!)
            bool matching = false;
            for (size_t o = 0; o < welded; o++) {
                uint32_t index = weld_triangles[o];

                glm::vec3 test_position = weld_positions[o];
                glm::vec2 test_tex_coord = weld_tex_coords[o];
                glm::vec3 test_normal = weld_normals[o];

                if (position == test_position && tex_coord == test_tex_coord && normal == test_normal) {
                    weld_triangles.emplace_back(static_cast<uint32_t>(o));
                    matching = true;
                    break;
                }
            }

            if (!matching) {
                weld_triangles.emplace_back(welded);

                welded += 1;

                weld_positions.emplace_back(position);
                weld_tex_coords.emplace_back(tex_coord);
                weld_normals.emplace_back(normal);
            }
        }
#else
        for (size_t t = 0; t < unweld_triangles.size(); t++) {
            OBJTriangle triangle = unweld_triangles[t];

            glm::vec3 position = unweld_positions[triangle.v];
            glm::vec2 tex_coord = unweld_tex_coords[triangle.t];
            glm::vec3 normal = unweld_normals[triangle.n];

            weld_triangles.emplace_back(welded);

            welded += 1;

            weld_positions.emplace_back(position);
            weld_tex_coords.emplace_back(tex_coord);
            weld_normals.emplace_back(normal);
        }
#endif

        auto mesh = new StaticMeshAsset();

        mesh->set_position_data(weld_positions.data(), welded);
        mesh->set_normal_data(weld_normals.data(), welded);
        mesh->set_uv0_data(weld_tex_coords.data(), welded);

        mesh->set_triangle_data(weld_triangles.data(), weld_triangles.size());

        RenderServer::get_singleton()->populate_mesh_buffer(mesh);

        return cache_asset(name, mesh);
    }

    return nullptr;
}