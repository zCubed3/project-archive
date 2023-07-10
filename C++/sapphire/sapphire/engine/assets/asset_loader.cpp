#include "asset_loader.h"

#include <iostream>

#include <engine/assets/loaders/material_loader.h>
#include <engine/assets/loaders/obj_loader.h>
#include <engine/assets/loaders/texture_loader.h>

#include <engine/assets/asset.h>
#include <engine/assets/texture_asset.h>
#include <engine/assets/material_asset.h>

std::vector<AssetLoader *> AssetLoader::loaders = {};

void AssetLoader::load_placeholders() {

}

void AssetLoader::unload_placeholders() {

}

void AssetLoader::register_engine_asset_loaders() {
    register_loader<OBJLoader>();
    register_loader<MaterialLoader>();
    register_loader<TextureLoader>();

    //ClassRegistry::register_class<Asset>();
    //ClassRegistry::register_class<TextureAsset>();
    //ClassRegistry::register_class<MaterialAsset>();
}

void AssetLoader::release_cache() {
    for (auto& pair: asset_cache) {
        if (pair.second.unique()) {
            pair.second.reset();
        } else { // Yes I am aware this isn't good to do, but I have no other option
            delete pair.second.get();
        }
    }
}

std::shared_ptr<Asset> AssetLoader::cache_asset(const std::string& name, Asset* p_asset) {
    auto iter = asset_cache.find(name);

    // TODO: Not replace assets?
    if (iter != asset_cache.end()) {
        iter->second = std::shared_ptr<Asset>(p_asset);
        return iter->second;
    } else {
        std::shared_ptr<Asset> asset = std::shared_ptr<Asset>(p_asset);
        asset_cache.emplace(name, asset);
        return asset;
    }
}

std::shared_ptr<Asset> AssetLoader::get_asset(const std::string& name) {
    auto iter = asset_cache.find(name);

    if (iter != asset_cache.end()) {
        return iter->second;
    } else {
        return nullptr;
    }
}

std::shared_ptr<Asset> AssetLoader::load_asset(const std::string &path) {
    // TODO: Make this smarter and safer
    size_t last_period = path.find_last_of('.');
    std::string extension = path.substr(last_period + 1);

    for (AssetLoader *loader: loaders) {
        if (loader != nullptr) {
            for (const std::string &expected: loader->get_extensions()) {
                if (extension == expected) {
                    return loader->load_from_path(path, extension);
                }
            }
        }
    }

    return nullptr;
}

void AssetLoader::load_all_placeholders() {
    for (AssetLoader *loader: loaders) {
        if (loader != nullptr) {
            loader->load_placeholders();
        }
    }
}

void AssetLoader::unload_all_assets() {
    for (AssetLoader *loader: loaders) {
        if (loader != nullptr) {
            loader->release_cache();
            loader->unload_placeholders();
        }
    }
}
