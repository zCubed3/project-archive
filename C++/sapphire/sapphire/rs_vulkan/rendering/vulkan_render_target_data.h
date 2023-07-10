#ifndef SAPPHIRE_VULKAN_RENDER_TARGET_DATA_H
#define SAPPHIRE_VULKAN_RENDER_TARGET_DATA_H

#include <vector>
#include <vulkan/vulkan.h>

#include <engine/rendering/render_target_data.h>

#include <val/render_targets/val_render_target.h>

class ValImageRenderTarget;
class VulkanTexture;
class TextureRenderTarget;

class VulkanRenderTargetData : public RenderTargetData {
public:
    VulkanTexture *color_texture = nullptr;
    VulkanTexture *depth_texture = nullptr;

    ValRenderTarget *val_render_target = nullptr;

    VulkanRenderTargetData(ValRenderTarget *p_val_render_target);
    ~VulkanRenderTargetData() override;

    void resize(int width, int height, RenderTarget* p_target) override;
    void setup_texture_rt(TextureRenderTarget *p_target, ValImageRenderTarget *p_val_target);

    Texture *get_color_texture() override;
    Texture *get_depth_texture() override;
};

#endif
