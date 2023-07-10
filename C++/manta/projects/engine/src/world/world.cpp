#include "world.hpp"

#include "actor.hpp"

#include <iostream>
#include <queue>

namespace Manta {
    void World::AddActor(Actor *actor) {
        actors.emplace_back(actor);
    }

    // TODO: Child actors and so on
    Actor *World::FindActor(const std::string &path) {
        for (Actor* actor : actors) {
            if (actor->name == path)
                return actor;
        }

        return nullptr;
    }

    void World::Update(EngineContext* engine) {
        std::queue<size_t> invalid_actors;

        for (auto a = 0; a < actors.size(); a++) {
            if (actors[a] == nullptr) {
                invalid_actors.emplace(a);
                continue;
            }

            actors[a]->Update(this, engine);
        }

        size_t off = 0;
        while (!invalid_actors.empty()) {
            auto idx = invalid_actors.front();
            actors.erase(actors.begin() + (idx - off));
            off += 1;

#ifdef DEBUG
            std::cout << "Disposing of nullptr actor at (" << idx << " - " << off << ")!" << std::endl;
#endif

            invalid_actors.pop();
        }
    }
}