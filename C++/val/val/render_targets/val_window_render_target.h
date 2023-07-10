#ifndef VAL_WINDOW_RENDER_TARGET_H
#define VAL_WINDOW_RENDER_TARGET_H

#include <vector>
#include <vulkan/vulkan.h>

#include <val/render_targets/val_render_target.h>

typedef struct SDL_Window SDL_Window;

namespace VAL {
    class ValInstance;
    class ValImage;
    class ValRenderPass;

    // TODO: Properly support multiple windows
    // TODO: Separate this into a framebuffer type?
    // TODO: Align this and various other types with Val*CreateInfo structure
    class ValWindowRenderTarget : public ValRenderTarget {
    protected:
        VkExtent2D get_extent(ValInstance *p_val_instance) override;
        VkFramebuffer get_framebuffer(ValInstance *p_val_instance) override;
        bool get_wait_for_image() override;

    public:
        VkSurfaceKHR vk_surface = nullptr;
        VkSwapchainKHR vk_swapchain = nullptr;
        VkSurfaceCapabilitiesKHR vk_capabilities;
        VkExtent2D vk_extent;
        uint32_t vk_frame_index = 0;

        SDL_Window *sdl_window;

        std::vector<VkImage> vk_swapchain_images;
        std::vector<VkImageView> vk_swapchain_image_views;
        std::vector<VkFramebuffer> vk_swapchain_framebuffers;

        ValImage *val_depth_image = nullptr;

        ValWindowRenderTarget(ValRenderTargetCreateInfo *p_create_info, ValInstance *p_val_instance);

        bool create_swapchain(ValRenderPass *p_val_render_pass, ValInstance *p_val_instance);

        bool present_queue(ValInstance *p_val_instance);

        void release(ValInstance *p_val_instance) override;
    };
}

#endif
