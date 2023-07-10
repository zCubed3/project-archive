#include "opengl4_mesh_buffer.h"

#include <gtc/matrix_transform.hpp>

#include <engine/assets/mesh_asset.h>
#include <engine/rendering/buffers/object_buffer.h>
#include <engine/rendering/material.h>
#include <engine/rendering/render_server.h>
#include <engine/rendering/render_target.h>
#include <engine/scene/world.h>

#include <glad/glad.h>

#include <rs_opengl4/rendering/opengl4_graphics_buffer.h>
#include <rs_opengl4/rendering/opengl4_render_server.h>
#include <rs_opengl4/rendering/opengl4_shader.h>

// TODO: Generate one engine default VAO
OpenGL4MeshBuffer::OpenGL4MeshBuffer(MeshAsset *p_mesh_asset) {
    // TODO: Allow creating mesh buffers without mesh assets?
    // TODO: Other types of mesh buffer for instancing, skinning, etc?
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &mbo);

    // TODO: Updating buffers post-construction
    // TODO: Optional channels?
    Vertex *vertices = new Vertex[p_mesh_asset->get_vertex_count()];

    uint32_t *triangles = p_mesh_asset->get_triangle_data(nullptr);
    glm::vec3 *positions = p_mesh_asset->get_position_data(nullptr);
    glm::vec3 *normals = p_mesh_asset->get_normal_data(nullptr);
    glm::vec2 *tex_coords = p_mesh_asset->get_uv0_data(nullptr);
    //glm::vec4 *tangents = p_mesh_asset->get_tangent_data(nullptr);

    tri_count = p_mesh_asset->get_triangle_count();

    for (uint32_t v = 0; v < p_mesh_asset->get_vertex_count(); v++) {
        if (positions != nullptr) {
            vertices[v].position = positions[v];
        }

        if (normals != nullptr) {
            vertices[v].normal = normals[v];
        }

        if (tex_coords != nullptr) {
            vertices[v].uv0 = tex_coords[v];
        }

        /*
        if (tangents != nullptr) {
            vertices[v].tangent = tangents[v];
        }
         */
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, mbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mbo);

    uint32_t vbo_size = sizeof(Vertex) * p_mesh_asset->get_vertex_count();
    uint32_t ibo_size = sizeof(uint32_t) * p_mesh_asset->get_triangle_count();

    glBufferData(GL_ARRAY_BUFFER, vbo_size + ibo_size, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vbo_size, vertices);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, vbo_size, ibo_size, triangles);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    const size_t vertex_size = sizeof(Vertex);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_size, nullptr);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertex_size, (void *) (sizeof(float) * 3));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vertex_size, (void *) (sizeof(float) * 6));

    sub_ibo_offset = vbo_size;

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    delete[] vertices;
}

void OpenGL4MeshBuffer::draw(ObjectBuffer *p_object_buffer, std::shared_ptr<Material> p_material) {
    // Get the current render target
    const OpenGL4RenderServer *rs_opengl4 = reinterpret_cast<const OpenGL4RenderServer *>(RenderServer::get_singleton());
    RenderTarget *current_target = rs_opengl4->get_current_target();

    // TODO: Move this to the render server
    //uint32_t view_handle = gl_shader->get_uniform_block("SAPPHIRE_VIEW_DATA");
    //uint32_t object_handle = gl_shader->get_uniform_block("SAPPHIRE_OBJECT_DATA");

    //OpenGL4GraphicsBuffer *view_ubo = reinterpret_cast<OpenGL4GraphicsBuffer *>(current_target->view_buffer->buffer);
    //OpenGL4GraphicsBuffer *object_ubo = reinterpret_cast<OpenGL4GraphicsBuffer *>(p_object_buffer->buffer);

    /*
    glUniformBlockBinding(gl_shader->shader_handle, view_handle, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, view_ubo->buffer_handle);

    glUniformBlockBinding(gl_shader->shader_handle, object_handle, 1);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, object_ubo->buffer_handle);

    glUseProgram(gl_shader->shader_handle);
     */

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, mbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mbo);

    glDrawElements(GL_TRIANGLES, tri_count, GL_UNSIGNED_INT, (void*)sub_ibo_offset);

    glBindVertexArray(0);
}