#include "static_mesh_asset.h"

#include <cstring>

#include <engine/rendering/buffers/mesh_buffer.h>
#include <engine/rendering/render_server.h>

void StaticMeshAsset::create_primitives() {
    if (primitive_quad == nullptr) {
        primitive_quad = new StaticMeshAsset();

        uint32_t quad_triangles[] = {
                2, 1, 0, 1, 2, 3
        };

        glm::vec3 quad_positions[] = {
                {-1, -1, 0},
                {-1, 1, 0},
                {1, -1, 0},
                {1, 1, 0},
        };

        glm::vec3 quad_normals[] = {
                {0, 0, 1},
                {0, 0, 1},
                {0, 0, 1},
                {0, 0, 1},
        };

        glm::vec2 quad_uvs[] = {
                {0, 0},
                {0, 1},
                {1, 0},
                {1, 1},
        };

        primitive_quad->set_triangle_data(quad_triangles, 6);
        primitive_quad->set_position_data(quad_positions, 4);
        primitive_quad->set_normal_data(quad_normals, 4);
        primitive_quad->set_uv0_data(quad_uvs, 4);

        RenderServer::get_singleton()->populate_mesh_buffer(primitive_quad);
    }

    if (primitive_cube == nullptr) {
        primitive_cube = new StaticMeshAsset();

        uint32_t triangles[] = {
                2, 1, 0, 1, 2, 3
        };

        glm::vec3 positions[] = {
                {-1, -1, 0},
                {-1, 1, 0},
                {1, -1, 0},
                {1, 1, 0},
        };

        glm::vec3 normals[] = {
                {0, 0, 1},
                {0, 0, 1},
                {0, 0, 1},
                {0, 0, 1},
        };

        glm::vec2 uvs[] = {
                {0, 0},
                {0, 1},
                {1, 0},
                {1, 1},
        };

        primitive_cube->set_triangle_data(triangles, 6);
        primitive_cube->set_position_data(positions, 4);
        primitive_cube->set_normal_data(normals, 4);
        primitive_cube->set_uv0_data(uvs, 4);

        RenderServer::get_singleton()->populate_mesh_buffer(primitive_cube);
    }
}

StaticMeshAsset *StaticMeshAsset::primitive_quad = nullptr;
StaticMeshAsset *StaticMeshAsset::primitive_cube = nullptr;

void StaticMeshAsset::set_position_data(glm::vec3 *data, size_t length) {
    delete[] position_data;

    position_data = new glm::vec3[length];
    memcpy(position_data, data, sizeof(glm::vec3) * length);

    // TODO: Validate vertex count and data sizes!
    vertex_count = length;
}

void StaticMeshAsset::set_normal_data(glm::vec3 *data, size_t length) {
    delete[] normal_data;

    normal_data = new glm::vec3[length];
    memcpy(normal_data, data, sizeof(glm::vec3) * length);

    // TODO: Validate vertex count and data sizes!
    vertex_count = length;
}

void StaticMeshAsset::set_uv0_data(glm::vec2 *data, size_t length) {
    delete[] uv0_data;

    uv0_data = new glm::vec2[length];
    memcpy(uv0_data, data, sizeof(glm::vec2) * length);

    // TODO: Validate vertex count and data sizes!
    vertex_count = length;
}

void StaticMeshAsset::set_tangent_data(glm::vec4 *data, size_t length) {
    delete[] tangent_data;

    tangent_data = new glm::vec4[length];
    memcpy(tangent_data, data, sizeof(glm::vec4) * length);

    // TODO: Validate vertex count and data sizes!
    vertex_count = length;
}

void StaticMeshAsset::set_triangle_data(uint32_t *data, size_t length) {
    delete[] triangle_data;

    triangle_data = new uint32_t[length];
    memcpy(triangle_data, data, sizeof(uint32_t) * length);

    // TODO: Validate vertex count and data sizes!
    triangle_count = length;
}

glm::vec3 *StaticMeshAsset::get_position_data(size_t *p_length) {
    if (p_length != nullptr) {
        *p_length = vertex_count;
    }

    return position_data;
}

glm::vec3 *StaticMeshAsset::get_normal_data(size_t *p_length) {
    if (p_length != nullptr) {
        *p_length = vertex_count;
    }

    return normal_data;
}

glm::vec2 *StaticMeshAsset::get_uv0_data(size_t *p_length) {
    if (p_length != nullptr) {
        *p_length = vertex_count;
    }

    return uv0_data;
}

glm::vec4 *StaticMeshAsset::get_tangent_data(size_t *p_length) {
    if (p_length != nullptr) {
        *p_length = vertex_count;
    }

    return tangent_data;
}

uint32_t *StaticMeshAsset::get_triangle_data(size_t *p_length) {
    if (p_length != nullptr) {
        *p_length = triangle_count;
    }

    return triangle_data;
}

uint32_t StaticMeshAsset::get_vertex_count() {
    return static_cast<uint32_t>(vertex_count);
}

uint32_t StaticMeshAsset::get_triangle_count() {
    return static_cast<uint32_t>(triangle_count);
}

void StaticMeshAsset::render(ObjectBuffer *p_object_buffer, Material *p_material) {
    if (buffer != nullptr) {
        //buffer->draw(p_object_buffer, p_material);
    }
}

StaticMeshAsset *StaticMeshAsset::get_primitive(Primitive primitive) {
    switch (primitive) {
        default:
            return nullptr;

        case Primitive::PRIMITIVE_QUAD:
            return primitive_quad;
    }
}