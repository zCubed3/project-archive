#ifndef MANTA_VIEWPORT_HPP
#define MANTA_VIEWPORT_HPP

#include <world/transform.hpp>

#include <vector>

namespace Manta::Rendering {
    class RenderTarget;

    struct ViewportRect {
        uint32_t x = 0, y = 0;
        uint32_t width = 512, height = 512;
    };

    // A basic class describing a renderer viewport, a camera derives from this!
    class Viewport {
    public:
        ViewportRect rect {};
        float fov = 60, z_near = 0.001f, z_far = 100.0f;

        glm::vec3 clear_color = glm::vec3(0.1f, 0.1f, 0.1f);
        bool clear = true, clear_depth = true;

        glm::vec3 viewing_pos; // Used to tell the renderer where this viewport is seeing from!
        glm::mat4x4 view, perspective, eye;

        // Are we drawing into a buffer?
        RenderTarget* render_target = nullptr;

        void UpdateViewport();
    };
}

#endif
