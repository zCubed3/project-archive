#include "material_loader.h"

#include <fstream>

#include <engine/assets/material_asset.h>
#include <core/config/config_file.h>
#include <core/platforms/platform.h>
#include <engine/rendering/render_server.h>
#include <engine/rendering/material.h>
#include <engine/rendering/shader.h>

std::vector<std::string> MaterialLoader::get_extensions() {
    return {"semd", "bsemd"};
}

void MaterialLoader::release_cache() {
    AssetLoader::release_cache();
    Shader::release_cache();
}

std::shared_ptr<Asset> MaterialLoader::load_from_path(const std::string &path, const std::string& extension) {
    const RenderServer* rs_instance = RenderServer::get_singleton();

    if (rs_instance == nullptr) {
        return nullptr;
    }

    if (extension == "semd") {
        // SEMD files are just config files
        ConfigFile semd_file;
        semd_file.read_from_path(path);

        std::string sesd_path = semd_file.try_get_string("sSESDPath", "Material");
        if (!Platform::get_singleton()->file_exists(sesd_path)) {
            return nullptr;
        }

        ConfigFile sesd_file;
        sesd_file.read_from_path(sesd_path);

        // We then create a shader
        // Check if the referenced SESD file has been loaded before
        // TODO: Use smart pointers for asset references
        std::string sesd_name = sesd_file.try_get_string("sName", "Shader");

        if (sesd_name.empty()) {
            return nullptr;
        }

        std::shared_ptr<Shader> shader = Shader::get_cached_shader(sesd_name);
        if (shader == nullptr) {
            shader = std::shared_ptr<Shader>(rs_instance->create_shader());
            shader->make_from_sesd(&sesd_file);

            Shader::cache_shader(shader);
        }

        std::shared_ptr<Material> material = std::shared_ptr<Material>(rs_instance->create_material());
        material->shader = shader;
        material->make_from_semd(&semd_file);

        MaterialAsset *asset = new MaterialAsset();
        asset->material = material;

        // TODO: Use material name instead
        return cache_asset(sesd_name, asset);
    }

    if (extension == "bsemd") {
        // TODO
    }

    return nullptr;
}
