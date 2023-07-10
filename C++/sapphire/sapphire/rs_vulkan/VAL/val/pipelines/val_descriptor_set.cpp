#include "val_descriptor_set.hpp"

#include <val/val_instance.hpp>

#include <val/images/val_image.hpp>

namespace VAL {
    void ValDescriptorSet::write_binding(ValDescriptorSetWriteInfo *p_write_info) {
        VkDescriptorBufferInfo *buffer_info = nullptr;
        VkDescriptorImageInfo *image_info = nullptr;

        if (p_write_info->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
            buffer_info = new VkDescriptorBufferInfo();
            buffer_info->buffer = p_write_info->val_buffer->vk_buffer;

            if (p_write_info->val_buffer_section) {
                buffer_info->offset = p_write_info->val_buffer_section->get_offset();
                buffer_info->range = p_write_info->val_buffer_section->get_size();
            } else {
                buffer_info->offset = 0;
                buffer_info->range = static_cast<uint32_t>(p_write_info->val_buffer->size);
            }

            vk_buffer_infos.push_back(buffer_info);
        }

        // TODO: Writeable textures?
        if (p_write_info->type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
            image_info = new VkDescriptorImageInfo();
            image_info->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_info->imageView = p_write_info->val_image->vk_image_view;
            image_info->sampler = p_write_info->val_image->vk_sampler;

            vk_image_infos.push_back(image_info);
        }

        VkWriteDescriptorSet descriptor_write{};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = vk_descriptor_set;
        descriptor_write.dstBinding = p_write_info->binding_index;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = p_write_info->type;
        descriptor_write.descriptorCount = p_write_info->count;
        descriptor_write.pBufferInfo = buffer_info;
        descriptor_write.pImageInfo = image_info;
        descriptor_write.pTexelBufferView = nullptr;// Optional

        vk_write_sets.push_back(descriptor_write);
    }

    void ValDescriptorSet::write_binding_and_buffer(ValInstance *p_val_instance, ValDescriptorSetWriteInfo *p_write_info, void *data) {
        write_binding(p_write_info);

        p_write_info->val_buffer->write(data, 0, p_val_instance);
    }

    void ValDescriptorSet::update_set(ValInstance *p_val_instance) {
        vkUpdateDescriptorSets(p_val_instance->vk_device, static_cast<uint32_t>(vk_write_sets.size()), vk_write_sets.data(), 0, nullptr);

        // TODO: Not heap allocate as much?
        for (VkDescriptorBufferInfo *buffer_info: vk_buffer_infos) {
            delete buffer_info;
        }

        for (VkDescriptorImageInfo *image_info: vk_image_infos) {
            delete image_info;
        }

        vk_buffer_infos.clear();
        vk_image_infos.clear();
        vk_write_sets.clear();
    }

    void ValDescriptorSet::release(ValInstance *p_val_instance) {
        ValReleasable::release(p_val_instance);

        if (vk_descriptor_set != nullptr) {
            vkFreeDescriptorSets(p_val_instance->vk_device, p_val_instance->vk_descriptor_pool, 1, &vk_descriptor_set);
            vk_descriptor_set = nullptr;
        }
    }
}