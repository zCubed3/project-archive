#include "val_shader_module.hpp"

#include <val/val_instance.hpp>

namespace VAL {
    void ValShaderModule::release(ValInstance *p_val_instance) {
        ValReleasable::release(p_val_instance);

        if (vk_shader_module != nullptr) {
            vkDestroyShaderModule(p_val_instance->vk_device, vk_shader_module, nullptr);
            vk_shader_module = nullptr;
        }
    }

    ValShaderModule *ValShaderModule::create_shader_module(ValShaderModuleCreateInfo *p_create_info, ValInstance *p_val_instance) {
        if (p_create_info == nullptr) {
            return nullptr;
        }

        if (p_create_info->code.empty()) {
            return nullptr;
        }

        VkShaderModuleCreateInfo module_create_info{};
        module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        module_create_info.codeSize = p_create_info->code.size();
        module_create_info.pCode = reinterpret_cast<const uint32_t *>(p_create_info->code.data());

        VkShaderModule vk_shader_module = nullptr;
        if (vkCreateShaderModule(p_val_instance->vk_device, &module_create_info, nullptr, &vk_shader_module) != VK_SUCCESS) {
            return nullptr;
        }

        VkShaderStageFlagBits stage_flags;

        switch (p_create_info->stage) {
            case ValShaderModuleCreateInfo::STAGE_VERTEX:
                stage_flags = VK_SHADER_STAGE_VERTEX_BIT;
                break;

            case ValShaderModuleCreateInfo::STAGE_FRAGMENT:
                stage_flags = VK_SHADER_STAGE_FRAGMENT_BIT;
                break;
        }

        VkPipelineShaderStageCreateInfo stage_create_info{};
        stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_info.stage = stage_flags;
        stage_create_info.module = vk_shader_module;
        stage_create_info.pName = p_create_info->entry_point.c_str();

        ValShaderModule *val_module = new ValShaderModule();

        val_module->vk_shader_module = vk_shader_module;
        val_module->vk_stage_info = stage_create_info;
        val_module->stage = p_create_info->stage;

        return val_module;
    }
}