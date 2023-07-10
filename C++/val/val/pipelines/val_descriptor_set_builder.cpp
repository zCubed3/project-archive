#include "val_descriptor_set_builder.hpp"

#include <val/pipelines/val_descriptor_set_info.hpp>
#include <val/val_instance.hpp>

namespace VAL {
    void ValDescriptorSetBuilderSetInfo::push_binding(VkDescriptorSetLayoutBinding vk_binding) {
        bindings.push_back(vk_binding);

        layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
        layout_info.pBindings = bindings.data();
    }

    uint32_t ValDescriptorSetBuilderSetInfo::get_open_location(uint32_t vk_stage_flags) {
        uint32_t location = 0;
        bool collision = true;
        while (collision) {
            collision = false;

            for (VkDescriptorSetLayoutBinding binding: bindings) {
                if (binding.binding == location) {
                    if (binding.stageFlags & vk_stage_flags) {
                        collision = true;
                        break;
                    }
                }
            }

            if (collision) {
                location++;
            }
        }

        return location;
    }

    VkDescriptorSetLayout ValDescriptorSetBuilderSetInfo::build(ValInstance *p_val_instance) {
        VkDescriptorSetLayout vk_descriptor_layout = nullptr;

        if (vkCreateDescriptorSetLayout(p_val_instance->vk_device, &layout_info, nullptr, &vk_descriptor_layout) != VK_SUCCESS) {
            return nullptr;
        }

        return vk_descriptor_layout;
    }

    void ValDescriptorSetBuilder::push_set() {
        sets.emplace_back();
    }

    void ValDescriptorSetBuilder::push_binding(VkDescriptorType vk_type, uint32_t count, uint32_t vk_stage_flags) {
        if (sets.empty()) {
            push_set();
        }

        // We need to find a binding location within this set that doesn't collide with any other sets
        // TODO: Will this break shaders that expect a certain layout?
        uint32_t location = sets.back().get_open_location(vk_stage_flags);

        VkDescriptorSetLayoutBinding layout_binding{};
        layout_binding.binding = location;
        layout_binding.descriptorType = vk_type;
        layout_binding.descriptorCount = count;
        layout_binding.stageFlags = vk_stage_flags;
        layout_binding.pImmutableSamplers = nullptr;// TODO: Immutable samplers?

        sets.back().push_binding(layout_binding);
    }

    std::vector<ValDescriptorSetInfo *> ValDescriptorSetBuilder::build(ValInstance *p_val_instance) {
        std::vector<ValDescriptorSetInfo *> val_descriptor_sets;

        // TODO: Abstract descriptor pools?
        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = p_val_instance->vk_descriptor_pool;
        alloc_info.descriptorSetCount = 1;

        for (ValDescriptorSetBuilderSetInfo &set: sets) {
            VkDescriptorSetLayout vk_descriptor_set_layout = set.build(p_val_instance);

            // TODO: Error reporting
            if (vk_descriptor_set_layout == nullptr) {
                for (ValDescriptorSetInfo *val_descriptor_set: val_descriptor_sets) {
                    val_descriptor_set->release(p_val_instance);
                    delete val_descriptor_set;
                }

                return {};
            }

            alloc_info.pSetLayouts = &vk_descriptor_set_layout;

            VkDescriptorSet vk_descriptor_set;
            if (vkAllocateDescriptorSets(p_val_instance->vk_device, &alloc_info, &vk_descriptor_set) != VK_SUCCESS) {
                for (ValDescriptorSetInfo *val_descriptor_set: val_descriptor_sets) {
                    val_descriptor_set->release(p_val_instance);
                    delete val_descriptor_set;
                }

                return {};
            }

            ValDescriptorSet *val_descriptor_set = new ValDescriptorSet();
            val_descriptor_set->vk_descriptor_set = vk_descriptor_set;

            ValDescriptorSetInfo *val_descriptor_set_info = new ValDescriptorSetInfo();
            val_descriptor_set_info->val_descriptor_set = val_descriptor_set;
            val_descriptor_set_info->vk_descriptor_set_layout = vk_descriptor_set_layout;

            val_descriptor_sets.push_back(val_descriptor_set_info);
        }

        return val_descriptor_sets;
    }
    void ValDescriptorSetBuilder::push_pre_allocate(bool pre_allocate) {
        if (sets.empty()) {
            push_set();
        }

        sets.back().pre_allocate = pre_allocate;
    }
}