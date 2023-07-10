#ifndef VAL_PIPELINE_HPP
#define VAL_PIPELINE_HPP

#include <val/val_releasable.hpp>
#include <vulkan/vulkan.h>

namespace VAL {
    class ValPipeline : public ValReleasable {
    public:
        VkPipeline vk_pipeline = nullptr;
        VkPipelineLayout vk_pipeline_layout = nullptr;

        void release(ValInstance *p_val_instance) override;
    };
}

#endif
