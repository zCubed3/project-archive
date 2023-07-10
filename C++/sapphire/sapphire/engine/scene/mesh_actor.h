#ifndef SAPPHIRE_MESH_ACTOR_H
#define SAPPHIRE_MESH_ACTOR_H

#include <engine/scene/actor.h>

#include <memory>

class MeshAsset;
class MaterialAsset;
class MeshDrawObject;

// TODO: This is a temporary mesh renderer; replace it with something better!
class MeshActor : public Actor {
    REFLECT_CLASS(MeshActor, Actor)

public:
    MeshDrawObject* draw_object = nullptr;
    std::shared_ptr<MeshAsset> mesh_asset = nullptr;
    std::shared_ptr<MaterialAsset> material_asset = nullptr;

    bool entered_world = false;

    MeshActor();
    ~MeshActor() override;

    void draw(World *p_world) override;

    void on_enter_world(World *p_world) override;
};


#endif
