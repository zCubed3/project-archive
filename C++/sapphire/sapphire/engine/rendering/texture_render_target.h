#ifndef SAPPHIRE_TEXTURE_RENDER_TARGET_H
#define SAPPHIRE_TEXTURE_RENDER_TARGET_H

#include <engine/rendering/render_target.h>

class Texture;

class TextureRenderTarget : public RenderTarget {
protected:
    Rect rect = {};

public:
    enum UsageIntent {
        USAGE_INTENT_GENERAL,
        USAGE_INTENT_SHADOW
    };

    UsageIntent usage_intent = USAGE_INTENT_GENERAL;

    TextureRenderTarget(int width, int height, UsageIntent usage = USAGE_INTENT_GENERAL);

    void resize(int width, int height);
    Texture *get_color_texture();
    Texture *get_depth_texture();

    TargetType get_type() override;
    Rect get_rect() override;
};

#endif
