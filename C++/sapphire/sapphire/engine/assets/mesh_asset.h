#ifndef SAPPHIRE_MESH_ASSET_H
#define SAPPHIRE_MESH_ASSET_H

#include <engine/assets/asset.h>
#include <engine/scene/transform.h>

#include <memory>

#include <glm.hpp>

class Material;
class MeshBuffer;
class ObjectBuffer;

class MeshAsset : public Asset {
public:
    // TODO: Asset safety so dangling pointers can't happen
    std::shared_ptr<MeshBuffer> buffer = nullptr;

    ~MeshAsset() override;

    virtual glm::vec3 *get_position_data(size_t *p_length) = 0;
    virtual glm::vec3 *get_normal_data(size_t *p_length) = 0;
    virtual glm::vec2 *get_uv0_data(size_t *p_length) = 0;
    virtual glm::vec4 *get_tangent_data(size_t *p_length) = 0;
    virtual uint32_t *get_triangle_data(size_t *p_length) = 0;

    virtual uint32_t get_vertex_count() = 0;
    virtual uint32_t get_triangle_count() = 0;

    virtual void render(ObjectBuffer *p_object_buffer, Material *p_material) = 0;
};


#endif
