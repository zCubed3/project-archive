#ifndef MANTA_BEHAVIOR_HPP
#define MANTA_BEHAVIOR_HPP

#include <string>

namespace Manta {
    class Actor;
    class World;
    class EngineContext;

    class Behavior {
    public:
        bool enabled = true;

        virtual void Start(World *world, Actor* owner, EngineContext* engine);
        virtual bool Update(World *world, Actor* owner, EngineContext* engine);

        virtual void OnDisable(World *world, Actor* owner, EngineContext* engine);
        virtual void OnEnable(World *world, Actor* owner, EngineContext* engine);

        virtual std::string get_TypeId() = 0;

        bool IsNew();

        Actor* owner = nullptr;

    protected:
        bool is_new = true;
        bool last_enabled = true;
    };
}

#endif
