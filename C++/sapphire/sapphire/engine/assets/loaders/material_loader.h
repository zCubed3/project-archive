#ifndef SAPPHIRE_MATERIAL_LOADER_H
#define SAPPHIRE_MATERIAL_LOADER_H

#include <engine/assets/asset_loader.h>
#include <engine/typing/class_registry.h>

// Sapphire Engine Material Definition
// By default semd is treated as text, binary forms are "semdb"
class MaterialLoader : public AssetLoader {
    REFLECT_CLASS(MaterialLoader, AssetLoader);

protected:
    void release_cache() override;

public:
    std::vector<std::string> get_extensions() override;

    std::shared_ptr<Asset> load_from_path(const std::string &path, const std::string& extension) override;
};


#endif
