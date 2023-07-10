#include <defines.hpp>

#include "game/gamegamemodule.hpp"

using namespace Manta;
using namespace Manta::Game;

EXPORT GameModule* module_init() {
    return new GameGameModule();
}