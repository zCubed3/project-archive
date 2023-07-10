#include "lighting.hpp"

#include <GL/glew.h>

#include <world/actor.hpp>
#include <world/behaviors/light.hpp>

#include <math/fmath.hpp>

// Manta's lighting is based on the ZeroLabRenderer for Unity
// I have worked with it plenty in Unity and is a very well-made renderer!
// https://github.com/KevinComerford/zero_lab_renderer/

// TODO: Don't rely entirely on OpenGL?
namespace Manta::Rendering {
    void Lighting::CreateBuffer() {
        glGenBuffers(1, &handle);

        glBindBuffer(GL_UNIFORM_BUFFER, handle);
        glBufferData(GL_UNIFORM_BUFFER, BUFFER_SIZE, nullptr, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        UpdateBuffer();
    }

    void Lighting::Update() {
        int actual_count = 0;
        for (auto light : lights) {
            if (!light->enabled || !light->owner->enabled)
                continue;

            data[actual_count].color_w_intensity = glm::vec4(light->color, light->intensity);
            data[actual_count].position_w_type = glm::vec4(light->owner->transform.position, light->light_type);
            data[actual_count].direction_w_todo = light->owner->transform.local_to_world * glm::vec4(0, 0, 1, 0);
            data[actual_count].direction_w_todo.w = 0;

            if (light->light_type == LightBehavior::LightType::Spotlight) {
                float perc = FMath::Clamp(light->inner_perc, 0.0f, 100.0f) / 100.0f;
                float phi = FMath::Clamp(cosf(light->cone_angle * 0.5f * FMath::DEG_2_RAD), 0, 1);
                float theta = FMath::Clamp(cosf(light->cone_angle * perc * 0.5f * FMath::DEG_2_RAD), 0, 1);

                data[actual_count].cone_cosines = glm::vec4(theta, phi, 1.0f / FMath::Max(0.01f, theta - phi), 0);
            }

            actual_count++;
        }

        light_count = actual_count;
    }

    void Lighting::UpdateBuffer() {
        glBindBuffer(GL_UNIFORM_BUFFER, handle);

        uint32_t data_size = sizeof(LightData);
        for (int l = 0; l < MAX_LIGHT_COUNT; l++) {
            glBufferSubData(GL_UNIFORM_BUFFER, data_size * l, data_size, &data[l]);
        }

        glBufferSubData(GL_UNIFORM_BUFFER, data_size * MAX_LIGHT_COUNT, sizeof(float), &light_count);

        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}