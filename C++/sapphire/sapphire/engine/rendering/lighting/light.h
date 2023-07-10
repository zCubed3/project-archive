#ifndef SAPPHIRE_LIGHT_H
#define SAPPHIRE_LIGHT_H

#include <glm.hpp>

class TextureRenderTarget;
class GraphicsBuffer;

// TODO: Temporary, make lights local to worlds!
class World;
class RenderServer;

struct LightShadowData {
    glm::mat4 light_matrix;
    glm::vec4 light_position;
};

// TODO: Use an atlas!
class Light {
public:
    glm::vec3 sun_pos = {30, 30, 30};
    GraphicsBuffer* buffer = nullptr;
    TextureRenderTarget* shadow = nullptr;

    Light();
    ~Light();

    void render_shadows(RenderServer *p_render_server, World *p_world);
};

#endif
