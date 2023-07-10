#ifndef VAL_DESCRIPTOR_SET_BUILDER_HPP
#define VAL_DESCRIPTOR_SET_BUILDER_HPP

#include <vector>
#include <vulkan/vulkan.h>

#include <val/pipelines/val_descriptor_set_info.hpp>

namespace VAL {
    class ValInstance;

    class ValDescriptorSetBuilderSetInfo {
    protected:
        VkDescriptorSetLayoutCreateInfo layout_info{};
        std::vector<VkDescriptorSetLayoutBinding> bindings;

    public:
        bool pre_allocate = false;

        void push_binding(VkDescriptorSetLayoutBinding vk_binding);
        uint32_t get_open_location(uint32_t vk_stage_flags);

        VkDescriptorSetLayout build(ValInstance *p_val_instance);
    };

    class ValDescriptorSetBuilder {
    protected:
        std::vector<ValDescriptorSetBuilderSetInfo> sets;

    public:
        void push_set();
        void push_binding(VkDescriptorType vk_type, uint32_t count, uint32_t vk_stage_flags);

        // Determines if the current set is pre-allocated or is just a layout
        void push_pre_allocate(bool pre_allocate);

        std::vector<ValDescriptorSetInfo *> build(ValInstance *p_val_instance);
    };
}

#endif
