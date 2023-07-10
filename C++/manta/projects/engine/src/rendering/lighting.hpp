#ifndef MANTA_LIGHTING_HPP
#define MANTA_LIGHTING_HPP

#include <cstdint>
#include <vector>

#include <glm/vec4.hpp>

namespace Manta {
    class LightBehavior;
}

namespace Manta::Rendering {
    struct LightData {
        glm::vec4 color_w_intensity;
        glm::vec4 position_w_type;
        glm::vec4 direction_w_todo;
        glm::vec4 cone_cosines;
    };

    class Lighting {
    public:
        // Change this inside shaders too!
        #define MAX_LIGHT_COUNT 64
        const uint32_t BUFFER_SIZE = sizeof(LightData) * MAX_LIGHT_COUNT + sizeof(uint32_t);

        LightData data[MAX_LIGHT_COUNT];
        uint32_t light_count;

        std::vector<LightBehavior*> lights;

        void CreateBuffer();
        void Update();
        void UpdateBuffer();

        uint32_t handle;
    };
}

#endif
