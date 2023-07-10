#ifndef SAPPHIRE_OPENGL4_MESH_BUFFER_H
#define SAPPHIRE_OPENGL4_MESH_BUFFER_H

#include <cstdint>
#include <engine/rendering/buffers/mesh_buffer.h>

class MeshAsset;

class OpenGL4MeshBuffer : public MeshBuffer {
protected:
    uint32_t vao = -1;
    uint32_t mbo = -1;
    uint32_t sub_ibo_offset = 0;

    uint32_t tri_count;

public:
    OpenGL4MeshBuffer(MeshAsset *p_mesh_asset);

    void draw(ObjectBuffer *p_object_buffer, std::shared_ptr<Material> p_material) override;
};


#endif
