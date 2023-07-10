#ifndef MANTA_CVAR_HPP
#define MANTA_CVAR_HPP

#include "console.hpp"

#include <string>

namespace Manta::Console {
    class CVar : public CElem {
    public:
        std::string text;
        bool dirty = false; // Set to true when text changes to prevent excess parsing!
    };
}

#endif
