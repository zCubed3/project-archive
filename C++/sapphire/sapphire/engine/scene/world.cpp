#include "world.h"

#include <engine/scene/actor.h>
#include <engine/rendering/shader.h>
#include <engine/rendering/material.h>
#include <engine/rendering/objects/mesh_draw_object.h>

void World::add_actor(Actor *p_actor) {
    if (p_actor != nullptr) {
        root_actors.push_back(p_actor);

        p_actor->on_enter_world(this);
    }
}

// TODO: Clean up nullptr actors
void World::tick() {
    for (Actor* actor: root_actors) {
        if (actor == nullptr) {
            continue;
        }

        actor->tick(this);
    }
}

void World::draw() {
    for (Actor* actor: root_actors) {
        if (actor == nullptr) {
            continue;
        }

        actor->draw(this);
    }
}

void World::enqueue_mesh_draw_object(MeshDrawObject *object) {
    if (object != nullptr) {
        // Check if the material's shader exists first
        DrawObjectMaterialTree* target;
        std::shared_ptr<Shader> shader = nullptr;

        if (object->material != nullptr) {
            shader = object->material->shader;
        }

        auto shader_iter = draw_tree.find(shader);
        if (shader_iter == draw_tree.end()) {
            DrawObjectMaterialTree tree = {};
            draw_tree.emplace(shader, tree);

            target = &draw_tree[shader];
        } else {
            target = &shader_iter->second;
        }

        auto iter = target->find(object->material);

        if (iter == target->end()) {
            std::vector<MeshDrawObject *> objects{object};
            target->emplace(object->material, objects);
        } else {
            iter->second.push_back(object);
        }
    }
}
