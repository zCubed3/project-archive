#ifndef VAL_SHADER_MODULE_HPP
#define VAL_SHADER_MODULE_HPP

#include <string>
#include <val/val_releasable.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace VAL {
    struct ValShaderModuleCreateInfo {
        enum Stage {
            STAGE_VERTEX,
            STAGE_FRAGMENT
        };

        std::vector<char> code;
        Stage stage = Stage::STAGE_VERTEX;
        std::string entry_point = "main";
    };

    class ValShaderModule : public ValReleasable {
    public:
        VkShaderModule vk_shader_module = nullptr;
        VkPipelineShaderStageCreateInfo vk_stage_info;
        ValShaderModuleCreateInfo::Stage stage;

        void release(ValInstance *p_val_instance) override;

        static ValShaderModule *create_shader_module(ValShaderModuleCreateInfo *p_create_info, ValInstance *p_val_instance);
    };
}

#endif
