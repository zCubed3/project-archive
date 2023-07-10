#ifndef SAPPHIRE_STATIC_MESH_ASSET_H
#define SAPPHIRE_STATIC_MESH_ASSET_H

#include <engine/assets/mesh_asset.h>

#include <vector>

// Mesh that may be modified at runtime but isn't recommended for safety reasons
// It is usually sourced from a model file
// Changing data is destructive and unsafe!
class StaticMeshAsset : public MeshAsset {
protected:
    friend class OBJLoader;

    static void create_primitives();

    static StaticMeshAsset* primitive_quad;
    static StaticMeshAsset* primitive_cube;

    glm::vec3 *position_data = nullptr;
    glm::vec3 *normal_data = nullptr;
    glm::vec2 *uv0_data = nullptr;
    glm::vec4 *tangent_data = nullptr;
    uint32_t *triangle_data = nullptr;

    size_t vertex_count = 0;
    size_t triangle_count = 0;

public:
    enum Primitive {
        PRIMITIVE_QUAD,
        PRIMITIVE_CUBE
    };

    void set_vertex_count(size_t length);
    void set_triangle_count(size_t length);

    void set_position_data(glm::vec3 *data, size_t length);
    void set_normal_data(glm::vec3 *data, size_t length);
    void set_uv0_data(glm::vec2 *data, size_t length);
    void set_tangent_data(glm::vec4 *data, size_t length);
    void set_triangle_data(uint32_t *data, size_t length);

    glm::vec3 *get_position_data(size_t *p_length) override;
    glm::vec3 *get_normal_data(size_t *p_length) override;
    glm::vec2 *get_uv0_data(size_t *p_length) override;
    glm::vec4 *get_tangent_data(size_t *p_length) override;
    uint32_t *get_triangle_data(size_t *p_length) override;

    uint32_t get_vertex_count() override;
    uint32_t get_triangle_count() override;

    void render(ObjectBuffer *p_object_buffer, Material *p_material) override;

    // TODO: ProceduralMeshAsset for procedural primitives?
    static StaticMeshAsset *get_primitive(Primitive primitive);
};

#endif
