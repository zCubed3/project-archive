#ifndef VAL_RENDER_PASS_HPP
#define VAL_RENDER_PASS_HPP

#include <val/val_releasable.hpp>
#include <vulkan/vulkan.h>

namespace VAL {
    class ValRenderPass : public ValReleasable {
    public:
        VkRenderPass vk_render_pass = nullptr;

        void release(ValInstance *p_val_instance) override;
    };
}

#endif
