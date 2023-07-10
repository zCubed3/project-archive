#include "val_queue.hpp"

#include <val/val_instance.hpp>

namespace VAL {
    bool ValQueue::create_pool(ValInstance *p_val_instance) {
        uint32_t flags;
        switch (type) {
            default:
                flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                break;

            case QueueType::QUEUE_TYPE_TRANSFER:
                flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
                break;
        }

        VkCommandPoolCreateInfo pool_create_info{};

        pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_create_info.flags = flags;
        pool_create_info.queueFamilyIndex = family;

        // TODO: Error
        return vkCreateCommandPool(p_val_instance->vk_device, &pool_create_info, nullptr, &vk_pool) == VK_SUCCESS;
    }

    VkCommandBuffer ValQueue::allocate_buffer(ValInstance *p_val_instance) {
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = vk_pool;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer vk_buffer = nullptr;
        if (vkAllocateCommandBuffers(p_val_instance->vk_device, &alloc_info, &vk_buffer) != VK_SUCCESS) {
            return nullptr;
        }

        return vk_buffer;
    }

    void ValQueue::release(ValInstance *p_val_instance) {
        ValReleasable::release(p_val_instance);

        if (vk_pool != nullptr) {
            vkDestroyCommandPool(p_val_instance->vk_device, vk_pool, nullptr);
        }
    }
}