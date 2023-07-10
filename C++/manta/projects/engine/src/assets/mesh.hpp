#ifndef MANTA_MESH_HPP
#define MANTA_MESH_HPP

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <vector>
#include <string>

#include <data/meshes/obj.hpp>
#include <data/meshes/mbsm.hpp>
#include <data/meshes/mmdl.hpp>

namespace Manta {
    class Shader;
    class Viewport;
    class EngineContext;

    struct MeshBuffer;

    class Mesh {
    public:
        // We have 1 vertex type in the engine for now, later more will come with different data configs allowed inside them!
        struct Vertex {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec2 uv0;
            glm::vec4 tangent;

            bool operator==(const Vertex& v);
        };

        std::string name = "Unnamed";
        std::vector<uint32_t> indices;
        std::vector<Vertex> vertices;
        MeshBuffer* buffer;

        static Mesh* LoadFromFile(const std::string &path, EngineContext* engine);

    protected:
        void ReadOBJ(std::istream& source);
        void ReadBSM(std::istream& source);
        void ReadMMDL(std::istream &source);
    };

    // TODO: Recreating if we add dynamic mesh generation?
    struct MeshBuffer {
        virtual void Create(Mesh* mesh, EngineContext* engine) = 0;
        virtual void UpdateData(Mesh* mesh, EngineContext* engine) = 0;
    };
}

#endif
