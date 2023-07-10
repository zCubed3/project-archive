#ifndef SAPPHIRE_RENDER_TARGET_DATA_H
#define SAPPHIRE_RENDER_TARGET_DATA_H

class Texture;

class RenderTarget;

class RenderTargetData {
public:
    virtual ~RenderTargetData() = default;

    virtual void resize(int width, int height, RenderTarget* p_target) = 0;
    virtual Texture *get_color_texture() = 0;
    virtual Texture *get_depth_texture() = 0;
};

#endif
