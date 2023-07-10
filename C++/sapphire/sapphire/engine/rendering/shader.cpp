#include "shader.h"

#include <core/config/config_file.h>

#include <core/data/string_tools.h>

bool ShaderPass::make_from_sesd(ConfigFile *p_sesd_file) {
    write_depth = p_sesd_file->try_get_int("bWriteDepth", name, 1);

    std::string cull_string = p_sesd_file->try_get_string("sCullMode", name, "Back");

    if (StringTools::compare(cull_string, "Back")) {
        cull_mode = CULL_MODE_BACK;
    } else if (StringTools::compare(cull_string, "Front")) {
        cull_mode = CULL_MODE_FRONT;
    } else {
        cull_mode = CULL_MODE_NONE;
    }

    std::string depth_string = p_sesd_file->try_get_string("sDepthOp", name, "Less");

    if (StringTools::compare(depth_string, "Less")) {
        depth_op = DEPTH_OP_LESS;
    } else if (StringTools::compare(depth_string, "LessOrEqual")) {
        depth_op = DEPTH_OP_LESS_OR_EQUAL;
    } else if (StringTools::compare(depth_string, "Greater")) {
        depth_op = DEPTH_OP_GREATER;
    } else if (StringTools::compare(depth_string, "GreaterOrEqual")) {
        depth_op = DEPTH_OP_GREATER_OR_EQUAL;
    } else if (StringTools::compare(depth_string, "Equal")) {
        depth_op = DEPTH_OP_EQUAL;
    } else {
        depth_op = DEPTH_OP_ALWAYS;
    }

    return true;
}

std::unordered_map<std::string, std::shared_ptr<Shader>> Shader::shader_cache = {};

void Shader::release_cache() {
    for (auto pair: shader_cache) {
        //delete pair.second;
    }
}

std::shared_ptr<Shader> Shader::get_cached_shader(const std::string &name) {
    auto iter = shader_cache.find(name);

    if (iter != shader_cache.end()) {
        return iter->second;
    }

    return nullptr;
}

void Shader::cache_shader(const std::shared_ptr<Shader>& shader) {
    if (shader == nullptr) {
        return;
    }

    shader_cache.emplace(shader->name, shader);
}

ShaderPass *Shader::get_pass(const std::string &pass_name) {
    for (ShaderPass* pass: passes) {
        if (pass->name == pass_name) {
            return pass;
        }
    }

    return nullptr;
}

ShaderPass *Shader::bind_pass(const std::string &name) {
    ShaderPass *pass = get_pass(name);

    if (pass != nullptr) {
        pass->bind();
    }

    return pass;
}

bool Shader::make_from_sesd(ConfigFile *p_sesd_file) {
    if (p_sesd_file == nullptr) {
        return false;
    }

    // Parameters
    std::vector<std::string> texture_params = p_sesd_file->try_get_string_list("aTextureParameters", "Shader");

    if (!texture_params.empty() && texture_params.size() % 2 == 0) {
        for (int t = 0; t < texture_params.size(); t += 2) {
            ShaderParameter parameter {};
            parameter.location = atoi(texture_params[t].c_str());
            parameter.name = texture_params[t + 1];

            parameters.push_back(parameter);
        }
    }

    // Passes
    for (std::string pass: p_sesd_file->try_get_string_list("aPasses", "Shader")) {
        ShaderPass* shader_pass = create_shader_pass();
        shader_pass->name = pass;

        if (shader_pass->make_from_sesd(p_sesd_file)) {
            passes.push_back(shader_pass);
        }  else {
            delete shader_pass;
        }
    }

    return true;
}
