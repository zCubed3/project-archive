#ifndef SAPPHIRE_ACTOR_H
#define SAPPHIRE_ACTOR_H

#include <engine/typing/class_registry.h>
#include <engine/scene/transform.h>

class World;

class Actor {
    REFLECT_BASE_CLASS(Actor)

public:
    std::string name = "Actor";
    Transform transform;

    int id; // TODO: Non-randomized IDs?

    std::vector<Actor*> children;

    Actor();
    virtual ~Actor();

    virtual void tick(World* p_world);

    virtual void draw(World* p_world);

    // Called whenever an actor is added into the world heirarchy
    // Used to insert draw objects and other one-time data
    virtual void on_enter_world(World *p_world);

    // Called whenever an actor is removed from the world hierarchy
    virtual void on_exit_world(World *p_world);

#if defined(IMGUI_SUPPORT)
    virtual void draw_imgui(World *p_world);
#endif
};

#endif
