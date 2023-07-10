#include "gamegamemodule.hpp"

#include <iostream>

#include <assets/mesh.hpp>
#include <assets/shader.hpp>

#include <world/timing.hpp>
#include <world/world.hpp>
#include <world/actor.hpp>
#include <world/behaviors/camera.hpp>
#include <world/behaviors/light.hpp>

#include <rendering/lighting.hpp>
#include <rendering/renderer.hpp>

#include <data/engine_context.hpp>

#include <behaviors/freecam.hpp>

#include <input/axis.hpp>
#include <input/input_server.hpp>

namespace Manta::Game {
    void GameGameModule::Initialize(EngineContext *engine) {
        //Mesh* mesh = Mesh::LoadFromFile("test.bsm");
        //Mesh* obj_mesh = Mesh::LoadFromFile("test.obj");
        //Mesh* bsm_mesh = Mesh::LoadFromFile("test.bsm");
        Mesh* mmdl_mesh = Mesh::LoadFromFile("test.mmdl");

        Shader* shader = Shader::LoadFile("content/engine/shaders/lit.glsl");
        shader->Compile();

        world = new World();

        test_actor = new Actor("test");
        test_actor->meshes.emplace_back(mmdl_mesh);
        test_actor->shaders.emplace_back(shader);

        test_actor2 = new Actor("test2");
        test_actor2->meshes.emplace_back(mmdl_mesh);

        test_actor2->transform.position = glm::vec3(0, 0, -1);

        world->AddActor(test_actor);
        world->AddActor(test_actor2);

        camera_actor = new Actor("camera");
        camera_actor->transform.position = glm::vec3(0, 0, 1);
        camera = camera_actor->AddBehavior<CameraBehavior>();
        freecam = camera_actor->AddBehavior<FreecamBehavior>();

        camera_actor->transform.position.z = 1.5;
        camera_actor->transform.euler.y = 180;

        world->AddActor(camera_actor);

        auto light_actor = new Actor("light");
        light_actor->transform.position = glm::vec3(1, 1, 1);
        light = light_actor->AddBehavior<LightBehavior>();
        light->light_type = LightBehavior::LightType::Sun;

        world->AddActor(light_actor);

        auto vertical_axis = new Input::Axis();
        vertical_axis->AddKey(SDLK_w, 1.0f);
        vertical_axis->AddKey(SDLK_s, -1.0f);

        auto horizontal_axis = new Input::Axis();
        horizontal_axis->AddKey(SDLK_d, 1.0f);
        horizontal_axis->AddKey(SDLK_a, -1.0f);

        engine->input->AddAxis("horizontal", horizontal_axis);
        engine->input->AddAxis("vertical", vertical_axis);
    }

    void GameGameModule::Update(EngineContext* engine) {
        camera->fov = 60;
        camera->rect.width = engine->renderer->width;
        camera->rect.height = engine->renderer->height;

        auto spin = glm::vec3(0, engine->timing->delta_time, 0) * 20.0f;
        //test_actor->transform.euler += spin;

        world->Update(engine);

        engine->lighting->Update();
    }

    void GameGameModule::Draw(EngineContext* engine) {
        engine->renderer->DrawWorld(world, engine);
    }

    void GameGameModule::DrawGUI(EngineContext* engine) {

    }
}