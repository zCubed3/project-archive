#ifndef SAPPHIRE_ASSET_H
#define SAPPHIRE_ASSET_H

#include <engine/typing/class_registry.h>

class Asset {
    REFLECT_BASE_CLASS(Asset)

public:
    virtual ~Asset() = default;
};

#endif
