#ifndef VAL_IMAGE_RENDER_TARGET_H
#define VAL_IMAGE_RENDER_TARGET_H

#include <val/images/val_image.hpp>
#include <val/render_targets/val_render_target.h>

namespace VAL {
    class ValImageRenderTarget : public ValRenderTarget {
    protected:
        VkFramebuffer get_framebuffer(ValInstance *p_val_instance) override;
        VkExtent2D get_extent(ValInstance *p_val_instance) override;

        bool can_clear_color() override;

    public:
        VkFramebuffer vk_framebuffer = nullptr;
        ValImage *val_color_image = nullptr;
        ValImage *val_depth_image = nullptr;

        bool create_color = true;
        bool create_depth = true;

        VkExtent2D vk_extent{};
        VkFormat vk_format;
        ValRenderPass *val_render_pass = nullptr;

        ValImageRenderTarget(ValRenderTargetCreateInfo *p_create_info, ValInstance *p_val_instance);

        void recreate_target(ValInstance *p_val_instance);

        void resize(int width, int height, ValInstance *p_val_instance) override;

        void release(ValInstance *p_val_instance) override;
    };
}

#endif
