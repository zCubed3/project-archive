#ifndef SAPPHIRE_OBJ_LOADER_H
#define SAPPHIRE_OBJ_LOADER_H

#include <glm.hpp>

#include <vector>

#include <engine/assets/asset_loader.h>

class OBJLoader : public AssetLoader {
protected:
    void load_placeholders() override;

public:
    std::vector<std::string> get_extensions() override;

    std::shared_ptr<Asset> load_from_path(const std::string &path, const std::string& extension) override;
};

#endif
