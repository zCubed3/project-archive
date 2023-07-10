#include "material.h"

#include <engine/assets/asset_loader.h>
#include <core/config/config_file.h>

bool Material::make_from_semd(ConfigFile *p_semd_file) {
    ConfigFile::ConfigSection& param_section = p_semd_file->get_section("Parameters");

    // Ensure we didn't get the global section
    if (param_section.name != "Parameters") {
        return false;
    }

    for (ConfigFile::ConfigEntry& entry: param_section.entries) {
        // We need to make sure a parameter exists with this name
        for (Shader::ShaderParameter param: shader->parameters) {
            if (param.name == entry.name) {
                Shader::ShaderParameter param_override = param;

                // TODO: Actually abide by smart pointers
                if (param_override.type == Shader::SHADER_PARAMETER_TEXTURE) {
                    param_override.asset_ref = AssetLoader::load_asset(entry.string_value);
                }

                parameter_overrides.push_back(param_override);
            }
        }
    }

    return true;
}
