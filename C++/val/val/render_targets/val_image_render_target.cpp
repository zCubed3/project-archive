#include "val_image_render_target.h"

#include <val/pipelines/val_render_pass.hpp>
#include <val/val_instance.hpp>

namespace VAL {
    VkFramebuffer ValImageRenderTarget::get_framebuffer(ValInstance *p_val_instance) {
        return vk_framebuffer;
    }

    VkExtent2D ValImageRenderTarget::get_extent(ValInstance *p_val_instance) {
        return vk_extent;
    }

    bool ValImageRenderTarget::can_clear_color() {
        return create_color;
    }

    // TODO: Not use a constructor (for error catching)
    ValImageRenderTarget::ValImageRenderTarget(ValRenderTargetCreateInfo *p_create_info, ValInstance *p_val_instance) {
        vk_extent = p_create_info->extent;
        vk_format = p_create_info->format;
        val_render_pass = p_create_info->val_render_pass;

        create_color = p_create_info->create_color;
        create_depth = p_create_info->create_depth;

        recreate_target(p_val_instance);
    }

    void ValImageRenderTarget::recreate_target(ValInstance *p_val_instance) {
        if (val_color_image != nullptr) {
            val_color_image->release(p_val_instance);
            delete val_color_image;
            val_color_image = nullptr;
        }

        if (val_depth_image != nullptr) {
            val_depth_image->release(p_val_instance);
            delete val_depth_image;
            val_depth_image = nullptr;
        }

        if (vk_framebuffer != nullptr) {
            vkDestroyFramebuffer(p_val_instance->vk_device, vk_framebuffer, nullptr);
            vk_framebuffer = nullptr;
        }

        std::vector<VkImageView> attachments;

        if (create_color) {
            ValImageCreateInfo color_create_info{};
            color_create_info.format = vk_format;
            color_create_info.extent.width = vk_extent.width;
            color_create_info.extent.height = vk_extent.height;
            color_create_info.usage_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            color_create_info.aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
            color_create_info.sampler_info.mode_u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            color_create_info.sampler_info.mode_v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

            val_color_image = ValImage::create(&color_create_info, p_val_instance);

            attachments.push_back(val_color_image->vk_image_view);
        }

        if (create_depth) {
            ValImageCreateInfo depth_create_info{};
            depth_create_info.extent.width = vk_extent.width;
            depth_create_info.extent.height = vk_extent.height;
            depth_create_info.format = p_val_instance->present_info->vk_depth_format;
            depth_create_info.usage_flags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            depth_create_info.aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;

            val_depth_image = ValImage::create(&depth_create_info, p_val_instance);

            attachments.push_back(val_depth_image->vk_image_view);
        }

        // We need to create just a single framebuffer
        VkFramebufferCreateInfo framebuffer_create_info{};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass = val_render_pass->vk_render_pass;
        framebuffer_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebuffer_create_info.pAttachments = attachments.data();
        framebuffer_create_info.width = vk_extent.width;
        framebuffer_create_info.height = vk_extent.height;
        framebuffer_create_info.layers = 1;

        if (vkCreateFramebuffer(p_val_instance->vk_device, &framebuffer_create_info, nullptr, &vk_framebuffer) != VK_SUCCESS) {
            // TODO: Error, failed to create framebuffer!
        }
    }

    void ValImageRenderTarget::resize(int width, int height, ValInstance *p_val_instance) {
        vk_extent.width = width;
        vk_extent.height = height;

        recreate_target(p_val_instance);
    }

    void ValImageRenderTarget::release(ValInstance *p_val_instance) {
        ValReleasable::release(p_val_instance);

        if (val_color_image != nullptr) {
            val_color_image->release(p_val_instance);
            delete val_color_image;
            val_color_image = nullptr;
        }

        if (val_depth_image != nullptr) {
            val_depth_image->release(p_val_instance);
            delete val_depth_image;
            val_depth_image = nullptr;
        }

        if (vk_framebuffer != nullptr) {
            vkDestroyFramebuffer(p_val_instance->vk_device, vk_framebuffer, nullptr);
            vk_framebuffer = nullptr;
        }
    }
}