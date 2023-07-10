#ifndef VAL_LAYER_HPP
#define VAL_LAYER_HPP

#include <cstdint>
#include <vector>

namespace VAL {
    // TODO: Make extension sets for certain features? Ex: Raytracing?
    struct ValLayer {
        enum LayerFlags {
            LAYER_FLAG_OPTIONAL = 1
        };

        const char *name;
        uint32_t flags;
    };
}

#endif
