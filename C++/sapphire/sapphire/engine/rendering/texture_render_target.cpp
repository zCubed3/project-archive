#include "texture_render_target.h"

#include <engine/rendering/render_target_data.h>

TextureRenderTarget::TextureRenderTarget(int width, int height, UsageIntent usage_intent) {
    rect = {0, 0, width, height};
    this->usage_intent = usage_intent;
}

RenderTarget::TargetType TextureRenderTarget::get_type() {
    return RenderTarget::TargetType::TARGET_TYPE_TEXTURE;
}

void TextureRenderTarget::resize(int width, int height) {
    if (width != rect.width || height != rect.height) {
        rt_data->resize(width, height, this);

        rect.width = width;
        rect.height = height;
    }
}

Texture *TextureRenderTarget::get_color_texture() {
    return rt_data->get_color_texture();
}

Texture *TextureRenderTarget::get_depth_texture() {
    return rt_data->get_depth_texture();
}

Rect TextureRenderTarget::get_rect() {
    return rect;
}