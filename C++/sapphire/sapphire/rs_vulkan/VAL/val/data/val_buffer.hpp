#ifndef VAL_BUFFER_HPP
#define VAL_BUFFER_HPP

#include <vector>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <val/val_releasable.hpp>

namespace VAL {
    class ValInstance;
    class ValBuffer;

    struct ValBufferCreateInfo {
        size_t size;
        uint32_t usage;
        uint32_t flags;
    };

    class ValBufferSection {
    protected:
        friend class ValBuffer;

        size_t offset;
        size_t size;
        VmaVirtualAllocation vma_allocation;

    public:
        size_t get_offset() const {
            return offset;
        }

        size_t get_size() const {
            return size;
        }
    };

    class ValBuffer : public ValReleasable {
    protected:
        // TODO: Optimization: Leaving memory mapped in Vulkan isn't bad?
        uint8_t *mapped = nullptr;

        VmaVirtualBlock vma_block;

    public:
        VmaAllocation vma_allocation = nullptr;
        VkBuffer vk_buffer = nullptr;
        size_t size = 0;

        static ValBuffer *create_buffer(ValBufferCreateInfo *p_create_info, ValInstance *p_val_instance);

        void write(void *data, ValInstance *p_val_instance);
        void write(void *data, size_t offset, size_t size, ValInstance *p_val_instance);
        void write(void *date, ValBufferSection *p_section, ValInstance *p_val_instance);

        ValBufferSection *allocate_section(size_t section_size);
        void free_section(ValBufferSection *section);

        void release(ValInstance *p_val_instance) override;
    };
}

#endif
