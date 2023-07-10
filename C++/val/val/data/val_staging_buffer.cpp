#include "val_staging_buffer.hpp"

#include <val/data/val_buffer.hpp>
#include <val/val_instance.hpp>

namespace VAL {
    ValStagingBuffer::ValStagingBuffer(size_t size, uint32_t usage_flags, ValInstance *p_val_instance) {
        ValBufferCreateInfo create_info{};
        create_info.size = size;

        create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        val_staging_buffer = ValBuffer::create_buffer(&create_info, p_val_instance);

        create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage_flags;
        create_info.flags = 0;

        val_final_buffer = ValBuffer::create_buffer(&create_info, p_val_instance);
    }

    void ValStagingBuffer::write(void *data, ValInstance *p_val_instance, size_t offset, size_t size) const {
        val_staging_buffer->write(data, offset, size, p_val_instance);
    }

    void ValStagingBuffer::copy_buffer(VkCommandBuffer vk_command_buffer) const {
        VkBufferCopy copy_region{};
        copy_region.srcOffset = 0;
        copy_region.dstOffset = 0;
        copy_region.size = val_staging_buffer->size;

        vkCmdCopyBuffer(vk_command_buffer, val_staging_buffer->vk_buffer, val_final_buffer->vk_buffer, 1, &copy_region);
    }

    ValBuffer *ValStagingBuffer::finalize(ValInstance *p_val_instance) {
        val_staging_buffer->release(p_val_instance);
        val_staging_buffer = nullptr;

        ValBuffer *buffer = val_final_buffer;
        val_final_buffer = nullptr;

        return buffer;
    }

    void ValStagingBuffer::release(ValInstance *p_val_instance) {
        ValReleasable::release(p_val_instance);

        if (val_staging_buffer != nullptr) {
            val_staging_buffer->release(p_val_instance);
            delete val_staging_buffer;
        }

        if (val_final_buffer != nullptr) {
            val_final_buffer->release(p_val_instance);
            delete val_final_buffer;
        }
    }
}