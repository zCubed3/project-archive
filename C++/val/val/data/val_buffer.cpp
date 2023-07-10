#include "val_buffer.hpp"

#include <val/val_instance.hpp>

#ifdef DEBUG
#include <iostream>
#endif

namespace VAL {
    ValBuffer *ValBuffer::create_buffer(ValBufferCreateInfo *p_create_info, ValInstance *p_val_instance) {
        if (p_create_info == nullptr || p_val_instance == nullptr) {
            return nullptr;
        }

        VmaVirtualBlockCreateInfo block_info{};
        block_info.size = p_create_info->size;

        VmaVirtualBlock vma_block;

        if (vmaCreateVirtualBlock(&block_info, &vma_block) != VK_SUCCESS) {
            return nullptr;
        }

        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = p_create_info->size;
        buffer_info.usage = p_create_info->usage;

        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
        alloc_info.flags = p_create_info->flags;

        VkBuffer vk_buffer;
        VmaAllocation vma_allocation;

        VmaAllocationInfo allocation_info{};
        if (vmaCreateBuffer(p_val_instance->vma_allocator, &buffer_info, &alloc_info, &vk_buffer, &vma_allocation, &allocation_info) != VK_SUCCESS) {
            vmaDestroyVirtualBlock(vma_block);
            return nullptr;
        }

        ValBuffer *buffer = new ValBuffer();

        buffer->size = p_create_info->size;
        buffer->vma_block = vma_block;
        buffer->vk_buffer = vk_buffer;
        buffer->vma_allocation = vma_allocation;

#ifdef DEBUG
        std::cout << "Vulkan: 0x" << buffer << " created ValBuffer::vk_buffer" << std::endl;
        std::cout << "Vulkan: 0x" << buffer << " created ValBuffer::vma_allocation" << std::endl;
#endif

        return buffer;
    }

    void ValBuffer::write(void *data, ValInstance *p_val_instance) {
        write(data, 0, -1, p_val_instance);
    }

    void ValBuffer::write(void *data, size_t offset, size_t size, ValInstance *p_val_instance) {
        size_t actual = size;
        if (size == -1) {
            actual = this->size;
        }

        if (mapped == nullptr) {
            vmaMapMemory(p_val_instance->vma_allocator, vma_allocation, reinterpret_cast<void **>(&mapped));
        }

        memcpy(mapped + offset, data, actual);
    }

    void ValBuffer::write(void *data, ValBufferSection *p_section, ValInstance *p_val_instance) {
        if (p_section == nullptr) {
            return;
        }

        write(data, p_section->get_offset(), p_section->get_size(), p_val_instance);
    }

    ValBufferSection *ValBuffer::allocate_section(size_t section_size) {
        VmaVirtualAllocationCreateInfo alloc_info{};
        alloc_info.size = section_size;

        VmaVirtualAllocation allocation;
        size_t offset;
        if (vmaVirtualAllocate(vma_block, &alloc_info, &allocation, &offset) != VK_SUCCESS) {
            return nullptr;
        }

        ValBufferSection *section = new ValBufferSection();
        section->offset = offset;
        section->size = section_size;
        section->vma_allocation = allocation;

        // We expect the user to return their sub allocations

        return section;
    }

    void ValBuffer::free_section(ValBufferSection *section) {
        if (section != nullptr) {
            vmaVirtualFree(vma_block, section->vma_allocation);
        }
    }

    void ValBuffer::release(ValInstance *p_val_instance) {
        ValReleasable::release(p_val_instance);

        if (mapped != nullptr) {
            vmaUnmapMemory(p_val_instance->vma_allocator, vma_allocation);
            mapped = nullptr;
        }

        if (vma_allocation != nullptr) {
            vmaDestroyBuffer(p_val_instance->vma_allocator, vk_buffer, vma_allocation);
            vk_buffer = nullptr;
            vma_allocation = nullptr;

#ifdef DEBUG
            std::cout << "Vulkan: 0x" << this << " released ValBuffer::vk_buffer" << std::endl;
            std::cout << "Vulkan: 0x" << this << " released ValBuffer::vma_allocation" << std::endl;
#endif
        }
    }
}