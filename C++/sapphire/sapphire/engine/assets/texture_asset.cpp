#include "texture_asset.h"

#include <engine/rendering/texture.h>

TextureAsset::~TextureAsset() {
    delete texture;
}
