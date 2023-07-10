#ifndef SAPPHIRE_TEXTURE_LOADER_H
#define SAPPHIRE_TEXTURE_LOADER_H

#include <engine/assets/asset_loader.h>

class TextureAsset;

class TextureLoader : public AssetLoader {
    REFLECT_CLASS(TextureLoader, AssetLoader)

public:
    //std::shared_ptr<TextureAsset> load_texture(const std::string &path);

    std::vector<std::string> get_extensions() override;

    std::shared_ptr<Asset> load_from_path(const std::string &path, const std::string &extension) override;
};


#endif
