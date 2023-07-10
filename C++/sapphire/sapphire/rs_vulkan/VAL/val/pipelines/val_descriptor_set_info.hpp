#ifndef VAL_DESCRIPTOR_SET_INFO_HPP
#define VAL_DESCRIPTOR_SET_INFO_HPP

#include <vector>
#include <vulkan/vulkan.h>

#include <val/pipelines/val_descriptor_set.hpp>
#include <val/val_releasable.hpp>

namespace VAL {
    class ValDescriptorSetInfo : public ValReleasable {
    public:
        VkDescriptorSetLayout vk_descriptor_set_layout = nullptr;
        ValDescriptorSet *val_descriptor_set = nullptr;

        // Writes to the pre-allocated descriptor set
        void write_binding(ValDescriptorSetWriteInfo *p_write_info);

        // Writes to the pre-allocated descriptor set and target buffer
        void write_binding_and_buffer(ValInstance *p_val_instance, ValDescriptorSetWriteInfo *p_write_info, void *data);

        // Updates the pre-allocated descriptor set
        void update_set(ValInstance *p_val_instance);

        // Allocates a VkDescriptorSet instance from the contained layout
        ValDescriptorSet *allocate_set(ValInstance *p_val_instance);

        // Releases the set bound to this (useful if you need unique sets but want to free up pool space)
        void release_set(ValInstance *p_val_instance);

        void release(ValInstance *p_val_instance) override;
    };
}

#endif
