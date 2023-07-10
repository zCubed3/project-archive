#ifndef MANTA_AXIS_HPP
#define MANTA_AXIS_HPP

#include <vector>

#include <SDL2/SDL.h>

namespace Manta::Input {
    class InputServer;

    // For these to be updated correctly, place them inside a Manta::Input::InputServer object that is referenced by the engine context!
    class Axis {
    public:
        struct AxisKey {
            SDL_KeyCode code;
            float value;
        };

        float value, default_value = 0.0f;
        std::vector<AxisKey> keys;

        virtual void ProcessSDLEvent(SDL_Event* event);

        virtual void Update(InputServer* server);

        virtual void AddKey(SDL_KeyCode code, float value);
    };
}

#endif
