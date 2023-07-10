#include "mesh.hpp"

#include <rendering/viewport.hpp>

#include <assets/shader.hpp>

#include <rendering/lighting.hpp>
#include <rendering/renderer.hpp>

#include <GL/glew.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>

#include <world/timing.hpp>

#include <data/engine_context.hpp>

#include <math.h>

#include <rendering/newrenderer.hpp>

using namespace Manta::Data::Meshes;

namespace Manta {
    Mesh* Mesh::LoadFromFile(const std::string& path, EngineContext* engine) {
        std::ifstream file(path, std::ifstream::binary);

        if (!file.is_open())
            throw std::runtime_error("File wasn't found / couldn't be read!");

        // Try to grab the file extension
        // We look backwards in the path
        std::string extension;
        for (int c = path.size() - 1; c > 0; c--) {
            if (path.at(c) == '.')
                break;

            extension.insert(0, 1, path.at(c));
        }

        std::transform(extension.begin(), extension.end(), extension.begin(), tolower);

        Mesh* mesh = new Mesh();

        auto start = std::chrono::high_resolution_clock::now();

        if (extension == "obj")
            mesh->ReadOBJ(file);

        if (extension == "bsm")
            mesh->ReadBSM(file);

        if (extension == "mmdl")
            mesh->ReadMMDL(file);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << "Loading '" << mesh->name << "' (from '" << path << "') took " << duration.count() << "ms" << std::endl;

#ifdef DEBUG
        std::cout << "Model Info:" << std::endl;
        std::cout << "  Source Fmt: ";

        if (extension == "obj")
            std::cout << "Wavefront OBJ";

        if (extension == "bsm")
            std::cout << "MantaBSM (Binary Static Mesh)";

        if (extension == "mmdl")
            std::cout << "MantaMDL (MoDeL)";

        std::cout << std::endl;

        std::cout << "  Vertex Count: " << mesh->vertices.size() << std::endl;
        std::cout << "  Indice Count: " << mesh->indices.size() << std::endl;
#endif

        mesh->buffer = engine->renderer2->CreateMeshBuffer();
        mesh->buffer->Create(mesh, engine);

        return mesh;
    }

    void Mesh::ReadOBJ(std::istream& source) {
        auto obj = WavefrontOBJ::LoadFromStream(source);

        for (auto indice : obj->weld_indices)
            indices.emplace_back(indice);

        for (auto obj_vert : obj->weld_vertices) {
            Vertex vert {};

            for (int p = 0; p < 3; p++)
                vert.position[p] = obj_vert.position[p];

            for (int n = 0; n < 3; n++)
                vert.normal[n] = obj_vert.normal[n];

            for (int u = 0; u < 2; u++)
                vert.uv0[u] = obj_vert.uv[u];

            vertices.emplace_back(vert);
        }

        name = obj->name;

        delete obj;
    }

    void Mesh::ReadBSM(std::istream &source) {
        auto bsm = MantaBSM::LoadFromStream(source);

        name = bsm->name;

        for (auto indice : bsm->indices)
            indices.emplace_back(indice);

        for (auto bsm_vert : bsm->vertices) {
            Vertex vert {};

            for (int p = 0; p < 3; p++)
                vert.position[p] = bsm_vert.position[p];

            for (int u = 0; u < 2; u++)
                vert.uv0[u] = bsm_vert.uv[u];

            float yaw = bsm_vert.normal[0];
            float pitch = bsm_vert.normal[1];

            float cos_t = cosf(pitch);
            vert.normal = glm::vec3(sinf(yaw) * cos_t, sin(pitch), cosf(yaw) * cos_t);

            yaw = bsm_vert.tangent[0];
            pitch = bsm_vert.tangent[1];

            cos_t = cosf(pitch);
            vert.tangent = glm::vec4(sinf(yaw) * cos_t, sin(pitch), cosf(yaw) * cos_t, bsm_vert.tangent[2]);

            vertices.emplace_back(vert);
        }

        delete bsm;
    }

    void Mesh::ReadMMDL(std::istream &source) {
        auto mmdl = MantaMDL::LoadFromStream(source);

        for (auto channel : mmdl->channels) {
            uint32_t i = 0;
            for (auto element : channel->data) {
                if (channel->descriptor.hint == MantaMDL::ChannelHint::VERTEX) {
                    Vertex v{};

                    auto v3 = reinterpret_cast<MantaMDL::Vec3*>(element);
                    v.position = glm::vec3(v3->x, v3->y, v3->z);
                    vertices.emplace_back(v);
                }

                // TODO: Make this order independent
                if (channel->descriptor.hint == MantaMDL::ChannelHint::NORMAL) {
                    Vertex& v = vertices[i++];

                    auto v3 = reinterpret_cast<MantaMDL::Vec3*>(element);
                    v.normal = glm::vec3(v3->x, v3->y, v3->z);
                }

                if (channel->descriptor.hint == MantaMDL::ChannelHint::UV0) {
                    Vertex& v = vertices[i++];

                    auto v2 = reinterpret_cast<MantaMDL::Vec2*>(element);
                    v.uv0 = glm::vec2(v2->x, v2->y);
                }

                if (channel->descriptor.hint == MantaMDL::ChannelHint::TANGENT) {
                    Vertex& v = vertices[i++];

                    auto v4 = reinterpret_cast<MantaMDL::Vec4*>(element);
                    v.tangent = glm::vec4(v4->x, v4->y, v4->z, v4->w);
                }

                if (channel->descriptor.hint == MantaMDL::ChannelHint::INDEXER) {
                    auto i = reinterpret_cast<uint32_t*>(element);
                    indices.emplace_back(*i);
                }
            }
        }

        name = mmdl->name;

        delete mmdl;
    }

    bool Mesh::Vertex::operator==(const Vertex& v) {
        return position == v.position && normal == v.normal && uv0 == v.uv0;
    }
}
