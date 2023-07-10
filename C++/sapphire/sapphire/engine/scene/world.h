#ifndef SAPPHIRE_WORLD_H
#define SAPPHIRE_WORLD_H

#include <vector>
#include <unordered_map>
#include <memory>

class Shader;
class Material;
class MeshDrawObject;
class Actor;
class TextureAsset;

class World {
public:
    // TODO: Do I need to alias this?
    using DrawObjectMaterialTree = std::unordered_map<std::shared_ptr<Material>, std::vector<MeshDrawObject*>>;
    using DrawObjectTree = std::unordered_map<std::shared_ptr<Shader>, DrawObjectMaterialTree>;

    // TODO: Make timing not local to the world?
    float elapsed_time = 0;
    float delta_time = 0;

    std::string name;

    // TODO: Allow skyboxes to be an actor instead?
    std::shared_ptr<TextureAsset> skybox = nullptr;

    // TODO: Make draw trees local to a culling zone of sorts?
    DrawObjectTree draw_tree;

    std::vector<Actor*> root_actors;

    void add_actor(Actor* p_actor);

    void tick();
    void draw();

    void enqueue_mesh_draw_object(MeshDrawObject *p_object);
};

#endif
