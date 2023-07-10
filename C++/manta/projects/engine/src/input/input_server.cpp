#include "input_server.hpp"

#include "axis.hpp"

#include <iostream>

namespace Manta::Input {
    void InputServer::ProcessEvent(SDL_Event *event) {
        switch (event->type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                auto key = bound_keys.find(event->key.keysym.sym);

                if (key != bound_keys.end())
                    key->second = event->type == SDL_KEYDOWN;

                break;
            }

            default:
                break;

            case SDL_MOUSEWHEEL:
                mouse_scroll_y = event->wheel.y;
                break;
        }

        for (auto& axis : bound_axes)
            axis.second->ProcessSDLEvent(event);
    }

    void InputServer::UpdateBinds() {
        int x = 0, y = 0;
        SDL_GetRelativeMouseState(&x, &y);

        mouse_delta_x = x;
        mouse_delta_y = y;

        for (auto axis : bound_axes)
            axis.second->Update(this);
    }

    void InputServer::Reset() {
        mouse_scroll_y = 0;
    }

    void InputServer::AddAxis(const std::string& name, Axis *axis) {
#ifdef DEBUG
      std::cout << "Input: Binding axis '" << name << "'" << std::endl;
#endif

        bound_axes.emplace(name, axis);
    }
}