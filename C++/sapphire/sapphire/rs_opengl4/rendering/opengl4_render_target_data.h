#ifndef SAPPHIRE_OPENGL4_RENDER_TARGET_DATA_H
#define SAPPHIRE_OPENGL4_RENDER_TARGET_DATA_H

#include <cstdint>
#include <engine/rendering/render_target_data.h>

class OpenGL4Texture;

class OpenGL4RenderTargetData : public RenderTargetData {
public:
    uint32_t framebuffer_handle = -1;
    uint32_t depth_texture_handle = -1;
    uint32_t texture_handle = -1;

    OpenGL4Texture* color_texture = nullptr;
    OpenGL4Texture* depth_texture = nullptr;

    ~OpenGL4RenderTargetData() override;

    void resize(int width, int height, RenderTarget *p_target) override;

    Texture *get_color_texture() override;
    Texture *get_depth_texture() override;
};

#endif
