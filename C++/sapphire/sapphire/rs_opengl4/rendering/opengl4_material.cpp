#include "opengl4_material.h"

#include <engine/assets/texture_asset.h>

#include <rs_opengl4/rendering/opengl4_texture.h>

#include <glad/glad.h>

bool OpenGL4Material::bind_material(ShaderPass *p_shader_pass) {
    if (!shader->parameters.empty()) {
        // Whenever we apply a parameter we check if it has been overriden
        for (Shader::ShaderParameter& param: shader->parameters) {
            void* data = param.data;
            std::shared_ptr<Asset> asset_ref = param.asset_ref;

            for (Shader::ShaderParameter& override_param: parameter_overrides) {
                if (override_param.name == param.name) {
                    data = override_param.data;
                    asset_ref = override_param.asset_ref;
                    break;
                }
            }

            if (param.type == Shader::SHADER_PARAMETER_TEXTURE) {
                std::shared_ptr<TextureAsset> texture = std::reinterpret_pointer_cast<TextureAsset>(asset_ref);
                OpenGL4Texture* opengl4_texture = reinterpret_cast<OpenGL4Texture*>(texture->texture);

                // TODO: Texture bounds check
                glActiveTexture(GL_TEXTURE0 + param.location);
                glBindTexture(GL_TEXTURE_2D, opengl4_texture->handle);
            }
        }
    }

    return true;
}
