#ifndef VAL_RENDER_TARGET_H
#define VAL_RENDER_TARGET_H

#include <val/val_releasable.hpp>
#include <vulkan/vulkan.h>

typedef struct SDL_Window SDL_Window;

namespace VAL {
    class ValRenderPass;

    // All types of render target can be created via this info
    //
    // Note: Creating a window
    //  1) Set p_window to a valid SDL window pointer
    //  2) Set type = RENDER_TARGET_TYPE_WINDOW
    struct ValRenderTargetCreateInfo {
        enum RenderTargetType {
            RENDER_TARGET_TYPE_IMAGE,
            RENDER_TARGET_TYPE_WINDOW
        };

        RenderTargetType type = RenderTargetType::RENDER_TARGET_TYPE_IMAGE;

        // If type is RENDER_TARGET_TYPE_WINDOW, these values are ignored!
        VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
        bool create_depth = true;
        bool create_color = true;

        // If type is RENDER_TARGET_TYPE_IMAGE, these values are ignored!
        VkExtent2D extent;

        ValRenderPass *val_render_pass = nullptr;

        // TODO: Optional depth buffering?

#ifdef SDL_SUPPORT
        // Do we initialize the swapchain on creation?
        bool initialize_swapchain = true;

        SDL_Window *p_window = nullptr;
#endif
    };

    class ValRenderTarget : public ValReleasable {
    protected:
        // TODO: Expose this?
        virtual VkFramebuffer get_framebuffer(ValInstance *p_val_instance) = 0;
        virtual VkExtent2D get_extent(ValInstance *p_val_instance) = 0;
        virtual bool get_wait_for_image();

        virtual bool can_clear_color();
        virtual bool can_clear_depth();

    public:
        // Each render target has a command buffer
        // This is because usually a render target renders its own world
        VkCommandBuffer vk_command_buffer = nullptr;

        VkClearColorValue clear_color = {0, 0, 0, 1};
        VkClearDepthStencilValue clear_depth_stencil = {1.0F, 0};

        ~ValRenderTarget() = default;

        virtual bool begin_render(ValRenderPass *p_val_render_pass, ValInstance *p_val_instance);
        virtual bool end_render(ValInstance *p_val_instance);

        virtual void resize(int width, int height, ValInstance *p_val_instance);

        static ValRenderTarget *create_render_target(ValRenderTargetCreateInfo *p_create_info, ValInstance *p_val_instance);
    };
}

#endif
