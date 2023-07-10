#include "val_descriptor_set_info.hpp"

#include <val/val_instance.hpp>

namespace VAL {
    void ValDescriptorSetInfo::write_binding(ValDescriptorSetWriteInfo *p_write_info) {
        if (val_descriptor_set != nullptr) {
            val_descriptor_set->write_binding(p_write_info);
        }
    }

    void ValDescriptorSetInfo::write_binding_and_buffer(ValInstance *p_val_instance, ValDescriptorSetWriteInfo *p_write_info, void *data) {
        if (val_descriptor_set != nullptr) {
            val_descriptor_set->write_binding_and_buffer(p_val_instance, p_write_info, data);
        }
    }

    void ValDescriptorSetInfo::update_set(ValInstance *p_val_instance) {
        if (val_descriptor_set != nullptr) {
            val_descriptor_set->update_set(p_val_instance);
        }
    }

    // TODO: Wrap around VkDescriptorSet and allocate differently
    ValDescriptorSet *ValDescriptorSetInfo::allocate_set(ValInstance *p_val_instance) {
        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = p_val_instance->vk_descriptor_pool;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &vk_descriptor_set_layout;

        VkDescriptorSet unique_vk_descriptor_set = nullptr;
        if (vkAllocateDescriptorSets(p_val_instance->vk_device, &alloc_info, &unique_vk_descriptor_set) != VK_SUCCESS) {
            return nullptr;
        }

        ValDescriptorSet *val_descriptor_set = new ValDescriptorSet();
        val_descriptor_set->vk_descriptor_set = unique_vk_descriptor_set;

        return val_descriptor_set;
    }

    void ValDescriptorSetInfo::release_set(ValInstance *p_val_instance) {
        if (val_descriptor_set != nullptr) {
            val_descriptor_set->release(p_val_instance);
            delete val_descriptor_set;

            val_descriptor_set = nullptr;
        }
    }

    void ValDescriptorSetInfo::release(ValInstance *p_val_instance) {
        ValReleasable::release(p_val_instance);

        release_set(p_val_instance);

        if (vk_descriptor_set_layout != nullptr) {
            vkDestroyDescriptorSetLayout(p_val_instance->vk_device, vk_descriptor_set_layout, nullptr);
            vk_descriptor_set_layout = nullptr;
        }
    }
}