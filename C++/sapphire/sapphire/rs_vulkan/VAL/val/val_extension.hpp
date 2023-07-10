#ifndef VAL_EXTENSION_HPP
#define VAL_EXTENSION_HPP

#include <cstdint>
#include <vector>

#ifdef SDL_SUPPORT
typedef struct SDL_Window SDL_Window;
#endif

namespace VAL {
    // TODO: Make extension sets for certain features? Ex: Raytracing?
    struct ValExtension {
        enum ExtensionFlags {
            EXTENSION_FLAG_OPTIONAL = 1
        };

        const char *name;
        uint32_t flags;

#if defined(SDL_SUPPORT)
        // Returns SDL's required instance extensions (flagged as non optional)
        static std::vector<ValExtension> get_sdl_instance_extensions(SDL_Window *p_window);
#endif
    };
}

#endif
