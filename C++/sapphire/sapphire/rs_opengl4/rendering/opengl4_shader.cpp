#include "opengl4_shader.h"

#include <glad/glad.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include <gtc/type_ptr.hpp>

#include <core/config/config_file.h>

#include <rs_opengl4/rendering/opengl4_render_server.h>
#include <rs_opengl4/rendering/opengl4_texture.h>
#include <rs_opengl4/rendering/opengl4_graphics_buffer.h>

#include <engine/assets/texture_asset.h>
#include <engine/rendering/render_target.h>
#include <engine/rendering/objects/mesh_draw_object.h>
#include <engine/scene/world.h>

#include <preludes/frag_prelude.glsl.gen.h>
#include <preludes/vert_prelude.glsl.gen.h>

#include <shaders/error.glsl.gen.h>

std::string read_source(const std::string& path) {
    std::ifstream file(path);

    if (file.is_open()) {
        std::stringstream source;
        source << file.rdbuf();
        file.close();

        return source.str();
    }

    return "";
}

std::string OpenGL4ShaderPass::get_shader_error(uint32_t handle) {
    int status;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);

    if (status != GL_TRUE) {
        int log_size = 0;

        glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_size);

        char *log = new char[log_size];

        glGetShaderInfoLog(handle, log_size, nullptr, log);

        std::string log_str(log);
        delete[] log;

        return log_str;
    }

    return "";
}

std::string OpenGL4ShaderPass::get_program_error(uint32_t handle) {
    int status;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);

    if (status != GL_TRUE) {
        int log_size = 0;

        glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_size);

        char *log = new char[log_size];

        glGetProgramInfoLog(handle, log_size, nullptr, log);

        std::string log_str(log);
        delete[] log;

        return log_str;
    }

    return "";
}

uint32_t OpenGL4ShaderPass::create_shader(uint32_t type, const std::vector<const char *> &sources) {
    uint32_t handle = glCreateShader(type);
    glShaderSource(handle, static_cast<GLsizei>(sources.size()), sources.data(), nullptr);
    glCompileShader(handle);

    /*
    for (const auto src: sources) {
        std::cout << src << std::endl;
    }
    */

    return handle;
}

OpenGL4ShaderPass::~OpenGL4ShaderPass() {
    // TODO
}

bool OpenGL4ShaderPass::make_from_sesd(ConfigFile *p_sesd_file) {
    ConfigFile::ConfigSection &section = p_sesd_file->get_section(name);

    if (section.name == name) {
        ShaderPass::make_from_sesd(p_sesd_file);

        // We load all the sources provided by the semd
        std::vector<std::string> vert_source_paths = section.try_get_string_list("sVertGLSL");
        std::vector<std::string> frag_source_paths = section.try_get_string_list("sFragGLSL");

        // TODO: Catch when we can't read a file
        for (const std::string& path: vert_source_paths) {
            vert_code += read_source(path);
        }

        for (const std::string& path: frag_source_paths) {
            frag_code += read_source(path);
        }

        return true;
    }

    return false;
}

void OpenGL4ShaderPass::bind() {
    OpenGL4RenderServer *rs_opengl4 = reinterpret_cast<OpenGL4RenderServer *>(RenderServer::get_singleton());
    RenderTarget *current_target = rs_opengl4->get_current_target();

    // TODO: Move this into the render server?
    if (rs_opengl4->cull_mode != cull_mode) {
        rs_opengl4->cull_mode = cull_mode;

        switch (cull_mode) {
            case ShaderPass::CULL_MODE_BACK:
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                break;

            case ShaderPass::CULL_MODE_FRONT:
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT);
                break;

            case ShaderPass::CULL_MODE_NONE:
                glDisable(GL_CULL_FACE);
                break;
        }
    }

    if (rs_opengl4->depth_op != depth_op) {
        rs_opengl4->depth_op = depth_op;

        switch (depth_op) {
            case ShaderPass::DEPTH_OP_LESS:
                glDepthFunc(GL_LESS);
                break;

            case ShaderPass::DEPTH_OP_LESS_OR_EQUAL:
                glDepthFunc(GL_LEQUAL);
                break;

            case ShaderPass::DEPTH_OP_GREATER:
                glDepthFunc(GL_GREATER);
                break;

            case ShaderPass::DEPTH_OP_GREATER_OR_EQUAL:
                glDepthFunc(GL_GEQUAL);
                break;

            case ShaderPass::DEPTH_OP_ALWAYS:
                glDepthFunc(GL_ALWAYS);
                break;

            case ShaderPass::DEPTH_OP_EQUAL:
                glDepthFunc(GL_EQUAL);
                break;
        }
    }

    if (rs_opengl4->write_depth != write_depth) {
        rs_opengl4->write_depth = write_depth;
        glDepthMask(write_depth);
    }

    glUseProgram(shader_handle);

    uint32_t view_handle = get_uniform_block("SAPPHIRE_VIEW_DATA");

    if (view_handle != -1) {
        OpenGL4GraphicsBuffer *view_ubo = reinterpret_cast<OpenGL4GraphicsBuffer *>(current_target->view_buffer->buffer);

        glUniformBlockBinding(shader_handle, view_handle, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, view_ubo->buffer_handle);
    }

    // TODO: Temp
    if (current_target->world != nullptr) {
        std::shared_ptr<World> world = current_target->world;

        if (world->skybox != nullptr) {
            uint32_t cubemap_handle = get_uniform("_CUBEMAP");

            if (cubemap_handle != -1) {
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_CUBE_MAP, ((OpenGL4Texture*)world->skybox->texture)->handle);
            }
        }
    }
}

void OpenGL4ShaderPass::setup_object(MeshDrawObject *p_object) {
    uint32_t object_handle = get_uniform_block("SAPPHIRE_OBJECT_DATA");

    if (object_handle != -1) {
        OpenGL4GraphicsBuffer *object_ubo = reinterpret_cast<OpenGL4GraphicsBuffer *>(p_object->object_buffer->buffer);

        glUniformBlockBinding(shader_handle, object_handle, 1);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, object_ubo->buffer_handle);
    }
}

uint32_t OpenGL4ShaderPass::get_uniform(const std::string &var) {
    auto iter = uniform_cache.find(var);

    if (iter == uniform_cache.end()) {
        uint32_t loc = glGetUniformLocation(shader_handle, var.c_str());

        // Regardless of whether it exists we cache it
        // When setting is when the index is checked
        uniform_cache.emplace(var, loc);
        return loc;
    }

    return iter->second;
}

uint32_t OpenGL4ShaderPass::get_uniform_block(const std::string &var) {
    auto iter = uniform_cache.find(var);

    if (iter == uniform_cache.end()) {
        uint32_t loc = glGetUniformBlockIndex(shader_handle, var.c_str());

        // Regardless of whether it exists we cache it
        // When setting is when the index is checked
        uniform_cache.emplace(var, loc);
        return loc;
    }

    return iter->second;
}

void OpenGL4ShaderPass::create_vert_frag(OpenGL4Shader *p_shader) {
    uint32_t vert_shader = create_shader(GL_VERTEX_SHADER, {vert_code.c_str()});
    uint32_t frag_shader = create_shader(GL_FRAGMENT_SHADER, {frag_code.c_str()});

    std::string vert_error = get_shader_error(vert_shader);
    std::string frag_error = get_shader_error(frag_shader);

    bool failure = false;
    if (!vert_error.empty()) {
        std::cout << "Vertex program failed to compile!\n\n" << vert_error << std::endl;
        failure = true;
    }

    if (!frag_error.empty()) {
        std::cout << "Fragment program failed to compile!\n\n" << frag_error << std::endl;
        failure = true;
    }

    // TODO: ERROR CHECKING!
    uint32_t program = glCreateProgram();

    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);

    glLinkProgram(program);

    std::string link_error = get_program_error(program);
    if (!link_error.empty()) {
        std::cout << "Program failed to link!\n\n" << link_error << std::endl;
        failure = true;
    }

    glDetachShader(program, vert_shader);
    glDetachShader(program, frag_shader);

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    shader_handle = program;

    vert_code.clear();
    vert_code.resize(0);

    frag_code.clear();
    frag_code.resize(0);
}

OpenGL4Shader *OpenGL4Shader::error_shader = nullptr;

bool OpenGL4Shader::make_from_sesd(ConfigFile *p_sesd_file) {
    if (p_sesd_file == nullptr) {
        return false;
    }

    Shader::make_from_sesd(p_sesd_file);

    for (ShaderPass* pass: passes) {
        reinterpret_cast<OpenGL4ShaderPass*>(pass)->create_vert_frag(this);
    }

    return true;
}

ShaderPass *OpenGL4Shader::create_shader_pass() {
    return new OpenGL4ShaderPass();
}

void OpenGL4Shader::create_error_shader() {
    {
        std::string vert_source = VERT_PRELUDE_CONTENTS;
        std::string frag_source = FRAG_PRELUDE_CONTENTS;

        vert_source += ERROR_CONTENTS;
        frag_source += ERROR_CONTENTS;

        error_shader = new OpenGL4Shader();

        OpenGL4ShaderPass *shader_pass = new OpenGL4ShaderPass();
        shader_pass->name = "Error";
        shader_pass->vert_code = vert_source;
        shader_pass->frag_code = frag_source;
        shader_pass->create_vert_frag(error_shader);
    }
}