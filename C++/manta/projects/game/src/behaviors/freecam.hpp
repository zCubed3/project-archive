#ifndef MANTA_GAME_FREECAM_HPP
#define MANTA_GAME_FREECAM_HPP

#include <world/behavior.hpp>

namespace Manta::Game {
    class FreecamBehavior : public Behavior {
    public:
        bool Update(World *world, Actor *owner, EngineContext *engine) override;

        std::string get_TypeId() override;
    };
}

#endif
