#ifndef SAPPHIRE_ASSET_LOADER_H
#define SAPPHIRE_ASSET_LOADER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

#include <engine/typing/class_registry.h>

class Asset;

// Provides an abstract method of loading various assets
class AssetLoader {
    REFLECT_BASE_CLASS(AssetLoader);

protected:
    virtual void load_placeholders();
    virtual void unload_placeholders();

    virtual void release_cache();
    virtual std::shared_ptr<Asset> cache_asset(const std::string& name, Asset* p_asset);

public:
    static std::vector<AssetLoader *> loaders;

    std::unordered_map<std::string, std::shared_ptr<Asset>> asset_cache;

    virtual std::vector<std::string> get_extensions() = 0;

    virtual std::shared_ptr<Asset> get_asset(const std::string& name);

    virtual std::shared_ptr<Asset> load_from_path(const std::string &path, const std::string& extension) = 0;

    static std::shared_ptr<Asset> load_asset(const std::string &path);

    static void load_all_placeholders();
    static void unload_all_assets();

    static void register_engine_asset_loaders();

    template<class T>
    static void register_loader() {
        ClassRegistry::register_class<T>();
        loaders.push_back(new T());
    }
};

#endif
