#include "val_pipeline.hpp"

#include <val/val_instance.hpp>

namespace VAL {
    void ValPipeline::release(ValInstance *p_val_instance) {
        ValReleasable::release(p_val_instance);

        if (vk_pipeline_layout != nullptr) {
            vkDestroyPipelineLayout(p_val_instance->vk_device, vk_pipeline_layout, nullptr);
        }

        if (vk_pipeline != nullptr) {
            vkDestroyPipeline(p_val_instance->vk_device, vk_pipeline, nullptr);
        }
    }
}