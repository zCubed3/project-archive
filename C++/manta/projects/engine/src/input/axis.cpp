#include "axis.hpp"

#include "input_server.hpp"

namespace Manta::Input {
    // Does nothing by default but can be extended by derivatives!
    void Axis::ProcessSDLEvent(SDL_Event* server) {

    }

    void Axis::Update(InputServer *server) {
        value = default_value;

        // Axes are evaluated dependent on order!
        for (auto key : keys) {
            auto bind = server->bound_keys.find(key.code);

            if (bind != server->bound_keys.end())
                value = bind->second ? key.value : value;
            else // Try to bind the key in the server
                server->bound_keys.emplace(key.code, false);
        }
    }

    void Axis::AddKey(SDL_KeyCode code, float value) {
        AxisKey key {};
        key.code = code;
        key.value = value;

        keys.emplace_back(key);
    }
}