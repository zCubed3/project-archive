#include "val_render_target.h"

#include <val/pipelines/val_render_pass.hpp>
#include <val/render_targets/val_image_render_target.h>
#include <val/render_targets/val_window_render_target.h>
#include <val/val_instance.hpp>

namespace VAL {
    bool ValRenderTarget::get_wait_for_image() {
        return false;
    }

    bool ValRenderTarget::can_clear_color() {
        return true;
    }

    bool ValRenderTarget::can_clear_depth() {
        return true;
    }

    bool ValRenderTarget::begin_render(ValRenderPass *p_val_render_pass, ValInstance *p_val_instance) {
        if (vk_command_buffer == nullptr) {
            vk_command_buffer = p_val_instance->get_queue(ValQueue::QUEUE_TYPE_GRAPHICS).allocate_buffer(p_val_instance);
        }

        VkCommandBufferBeginInfo buffer_begin_info{};
        buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        buffer_begin_info.flags = 0;
        buffer_begin_info.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(vk_command_buffer, &buffer_begin_info) != VK_SUCCESS) {
            return false;
        }

        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = p_val_render_pass->vk_render_pass;
        render_pass_info.framebuffer = get_framebuffer(p_val_instance);

        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = get_extent(p_val_instance);

        std::vector<VkClearValue> clear_values;

        if (can_clear_color()) {
            VkClearValue clear_value{};
            clear_value.color = clear_color;

            clear_values.push_back(clear_value);
        }

        // TODO: Optional depth buffering?
        if (can_clear_depth()) {
            VkClearValue depth_clear_value{};
            depth_clear_value.depthStencil = clear_depth_stencil;

            clear_values.push_back(depth_clear_value);
        }

        render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        render_pass_info.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(vk_command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        return true;
    }

    bool ValRenderTarget::end_render(ValInstance *p_val_instance) {
        vkCmdEndRenderPass(vk_command_buffer);
        vkEndCommandBuffer(vk_command_buffer);

        // TODO: Make submitting at the end optional
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        if (get_wait_for_image()) {
            submit_info.waitSemaphoreCount = 1;
            submit_info.pWaitSemaphores = &p_val_instance->vk_image_available_semaphore;
            submit_info.pWaitDstStageMask = wait_stages;

            submit_info.signalSemaphoreCount = 1;
            submit_info.pSignalSemaphores = &p_val_instance->vk_render_finished_semaphore;
        } else {
            submit_info.waitSemaphoreCount = 0;
            submit_info.signalSemaphoreCount = 0;
        }

        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &vk_command_buffer;

        if (vkQueueSubmit(p_val_instance->get_queue(ValQueue::QUEUE_TYPE_GRAPHICS).vk_queue, 1, &submit_info, p_val_instance->vk_render_fence) != VK_SUCCESS) {
            return false;
        }

        if (!get_wait_for_image()) {
            vkQueueWaitIdle(p_val_instance->get_queue(ValQueue::QUEUE_TYPE_GRAPHICS).vk_queue);
        }

        return true;
    }

    void ValRenderTarget::resize(int width, int height, ValInstance *p_val_instance) {
    }

    ValRenderTarget *ValRenderTarget::create_render_target(ValRenderTargetCreateInfo *p_create_info, ValInstance *p_val_instance) {
        if (p_create_info == nullptr || p_val_instance == nullptr) {
            return nullptr;
        }

        if (p_create_info->type == ValRenderTargetCreateInfo::RENDER_TARGET_TYPE_IMAGE) {
            ValImageRenderTarget *image_target = new ValImageRenderTarget(p_create_info, p_val_instance);

            return image_target;
        }

        if (p_create_info->type == ValRenderTargetCreateInfo::RENDER_TARGET_TYPE_WINDOW) {
            ValWindowRenderTarget *window_target = new ValWindowRenderTarget(p_create_info, p_val_instance);

            if (p_create_info->initialize_swapchain) {
                window_target->create_swapchain(p_create_info->val_render_pass, p_val_instance);
            }

            return window_target;
        }

        return nullptr;
    }
}