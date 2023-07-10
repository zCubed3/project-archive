#include "texture_loader.h"

#include <core/platforms/platform.h>
#include <core/config/config_file.h>
#include <engine/assets/texture_asset.h>
#include <engine/rendering/render_server.h>
#include <engine/rendering/texture.h>

std::vector<std::string> TextureLoader::get_extensions() {
    return {"setd"};
}

std::shared_ptr<Asset> TextureLoader::load_from_path(const std::string &path, const std::string &extension) {
    if (!Platform::get_singleton()->file_exists(path)) {
        return nullptr;
    }

    const RenderServer* rs_instance = RenderServer::get_singleton();

    ConfigFile setd_file;

    if (!Platform::get_singleton()->file_exists(path)) {
        return nullptr;
    }

    setd_file.read_from_path(path);

    // TODO: sRGB textures
    // TODO: Not always assume the image can be read
    Texture* texture = rs_instance->create_texture();

    if (!texture->load_from_setd(&setd_file)) {
        delete texture;
        return nullptr;
    }

    TextureAsset *asset = new TextureAsset();
    asset->texture = texture;

    return cache_asset(path, asset);
}