#include "val_render_pass.hpp"

#include <val/val_instance.hpp>

namespace VAL {
    void ValRenderPass::release(ValInstance *p_val_instance) {
        ValReleasable::release(p_val_instance);

        if (vk_render_pass != nullptr) {
            vkDestroyRenderPass(p_val_instance->vk_device, vk_render_pass, nullptr);
            vk_render_pass = nullptr;
        }
    }
}