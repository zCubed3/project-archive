#include "val_window_render_target.h"

#include <SDL.h>
#include <SDL_vulkan.h>

#include <val/images/val_image.hpp>
#include <val/pipelines/val_render_pass.hpp>
#include <val/val_instance.hpp>

namespace VAL {
    VkExtent2D ValWindowRenderTarget::get_extent(ValInstance *p_val_instance) {
        return vk_extent;
    }

    VkFramebuffer ValWindowRenderTarget::get_framebuffer(ValInstance *p_val_instance) {
        vkAcquireNextImageKHR(
                p_val_instance->vk_device,
                vk_swapchain,
                UINT64_MAX,
                p_val_instance->vk_image_available_semaphore,
                VK_NULL_HANDLE,
                &vk_frame_index);

        return vk_swapchain_framebuffers[vk_frame_index];
    }

    bool ValWindowRenderTarget::get_wait_for_image() {
        return true;
    }

    ValWindowRenderTarget::ValWindowRenderTarget(ValRenderTargetCreateInfo *p_create_info, ValInstance *p_val_instance) {
        SDL_Vulkan_CreateSurface(p_create_info->p_window, p_val_instance->vk_instance, &vk_surface);
        sdl_window = p_create_info->p_window;
    }

    bool ValWindowRenderTarget::create_swapchain(ValRenderPass *p_val_render_pass, ValInstance *p_val_instance) {
        if (!p_val_instance->block_await) {
            p_val_instance->await_frame();
        }

        // TODO: Re-using val images?
        if (val_depth_image != nullptr) {
            val_depth_image->release(p_val_instance);
            delete val_depth_image;
        }

        // TODO: Move extent calculation elsewhere
        // TODO: SDL optional dependency
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(p_val_instance->vk_physical_device, vk_surface, &vk_capabilities);

        //
        // Hi-dpi aware extent
        //
        if (vk_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            vk_extent = vk_capabilities.currentExtent;
        } else {
            int width, height;
            SDL_Vulkan_GetDrawableSize(sdl_window, &width, &height);

            VkExtent2D actual_extent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)};

            actual_extent.width = std::min(std::max(actual_extent.width, vk_capabilities.minImageExtent.width), vk_capabilities.maxImageExtent.width);
            actual_extent.height = std::min(std::max(actual_extent.height, vk_capabilities.minImageExtent.height), vk_capabilities.maxImageExtent.height);

            vk_extent = actual_extent;
        }

        //
        // Swapchain
        //
        uint32_t image_count = vk_capabilities.minImageCount + 1;

        ValQueue graphics_queue = p_val_instance->get_queue(ValQueue::QueueType::QUEUE_TYPE_GRAPHICS);
        ValQueue present_queue = p_val_instance->get_queue(ValQueue::QueueType::QUEUE_TYPE_PRESENT);

        std::vector<uint32_t> device_queues{graphics_queue.family, present_queue.family};

        if (vk_capabilities.maxImageCount > 0 && image_count > vk_capabilities.maxImageCount) {
            image_count = vk_capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swapchain_create_info{};
        swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

        swapchain_create_info.surface = vk_surface;
        swapchain_create_info.minImageCount = image_count;
        swapchain_create_info.imageFormat = p_val_instance->present_info->vk_color_format;
        swapchain_create_info.imageColorSpace = p_val_instance->present_info->vk_color_space;
        swapchain_create_info.imageExtent = vk_extent;
        swapchain_create_info.imageArrayLayers = 1;
        swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        swapchain_create_info.preTransform = vk_capabilities.currentTransform;
        swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        swapchain_create_info.presentMode = p_val_instance->present_info->vk_present_mode;
        swapchain_create_info.clipped = VK_TRUE;

        swapchain_create_info.oldSwapchain = vk_swapchain;

        if (graphics_queue.family != present_queue.family) {
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchain_create_info.queueFamilyIndexCount = static_cast<uint32_t>(device_queues.size());
            swapchain_create_info.pQueueFamilyIndices = device_queues.data();
        } else {
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchain_create_info.queueFamilyIndexCount = 0;
            swapchain_create_info.pQueueFamilyIndices = nullptr;
        }

        if (vkCreateSwapchainKHR(p_val_instance->vk_device, &swapchain_create_info, nullptr, &vk_swapchain) != VK_SUCCESS) {
            // TODO: Error failed to create swapchain!
            return false;
        }

        //
        // Swapchain image views
        //

        // Clean up existing views
        for (VkImageView view: vk_swapchain_image_views) {
            vkDestroyImageView(p_val_instance->vk_device, view, nullptr);
        }

        // Clean up existing framebuffers
        for (VkFramebuffer framebuffer: vk_swapchain_framebuffers) {
            vkDestroyFramebuffer(p_val_instance->vk_device, framebuffer, nullptr);
        }

        vkGetSwapchainImagesKHR(p_val_instance->vk_device, vk_swapchain, &image_count, nullptr);
        vk_swapchain_images.resize(image_count);
        vkGetSwapchainImagesKHR(p_val_instance->vk_device, vk_swapchain, &image_count, vk_swapchain_images.data());

        vk_swapchain_image_views.resize(image_count);
        for (uint32_t i = 0; i < image_count; i++) {
            VkImageViewCreateInfo view_create_info{};
            view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

            view_create_info.image = vk_swapchain_images[i];
            view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view_create_info.format = p_val_instance->present_info->vk_color_format;

            view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            view_create_info.subresourceRange.baseMipLevel = 0;
            view_create_info.subresourceRange.levelCount = 1;
            view_create_info.subresourceRange.baseArrayLayer = 0;
            view_create_info.subresourceRange.layerCount = 1;

            if (vkCreateImageView(p_val_instance->vk_device, &view_create_info, nullptr, &vk_swapchain_image_views[i]) != VK_SUCCESS) {
                // TODO: Error failed to create image view!
                return false;
            }
        }

        //
        // Depth buffer
        //
        ValImageCreateInfo depth_create_info{};
        depth_create_info.extent.width = vk_extent.width;
        depth_create_info.extent.height = vk_extent.height;
        depth_create_info.format = p_val_instance->present_info->vk_depth_format;
        depth_create_info.usage_flags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        depth_create_info.aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;

        val_depth_image = ValImage::create(&depth_create_info, p_val_instance);

        vk_swapchain_framebuffers.resize(image_count);
        for (uint32_t i = 0; i < image_count; i++) {
            std::vector<VkImageView> attachments = {
                    vk_swapchain_image_views[i],
                    val_depth_image->vk_image_view};

            VkFramebufferCreateInfo framebuffer_create_info{};
            framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_create_info.renderPass = p_val_render_pass->vk_render_pass;
            framebuffer_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebuffer_create_info.pAttachments = attachments.data();
            framebuffer_create_info.width = vk_extent.width;
            framebuffer_create_info.height = vk_extent.height;
            framebuffer_create_info.layers = 1;

            if (vkCreateFramebuffer(p_val_instance->vk_device, &framebuffer_create_info, nullptr, &vk_swapchain_framebuffers[i]) != VK_SUCCESS) {
                // TODO: Error, failed to create framebuffer!
                return false;
            }
        }

        return true;
    }

    bool ValWindowRenderTarget::present_queue(ValInstance *p_val_instance) {
        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &p_val_instance->vk_render_finished_semaphore;

        present_info.swapchainCount = 1;
        present_info.pSwapchains = &vk_swapchain;
        present_info.pImageIndices = &vk_frame_index;

        vkQueuePresentKHR(p_val_instance->get_queue(ValQueue::QUEUE_TYPE_PRESENT).vk_queue, &present_info);

        return true;
    }

    void ValWindowRenderTarget::release(ValInstance *p_val_instance) {
        ValReleasable::release(p_val_instance);

        for (VkImageView view: vk_swapchain_image_views) {
            vkDestroyImageView(p_val_instance->vk_device, view, nullptr);
            view = nullptr;
        }

        for (VkFramebuffer framebuffer: vk_swapchain_framebuffers) {
            vkDestroyFramebuffer(p_val_instance->vk_device, framebuffer, nullptr);
            framebuffer = nullptr;
        }

        if (val_depth_image != nullptr) {
            val_depth_image->release(p_val_instance);
            delete val_depth_image;
        }

        vkDestroySwapchainKHR(p_val_instance->vk_device, vk_swapchain, nullptr);
        vkDestroySurfaceKHR(p_val_instance->vk_instance, vk_surface, nullptr);
    }
}