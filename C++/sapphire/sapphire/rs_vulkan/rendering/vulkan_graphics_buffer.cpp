#include "vulkan_graphics_buffer.h"

#include <rs_vulkan/rendering/vulkan_render_server.h>

#ifdef DEBUG
#include <iostream>
#endif

VulkanGraphicsBuffer::VulkanGraphicsBuffer(size_t size, UsageIntent usage) {
    const VulkanRenderServer *vulkan_rs = reinterpret_cast<const VulkanRenderServer*>(RenderServer::get_singleton());

    VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    switch (usage) {
        default:
            usage_flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;

        case GraphicsBuffer::USAGE_INTENT_LARGE_BUFFER:
            usage_flags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            break;
    }

    // TODO: Use SSBO pool
    val_buffer = vulkan_rs->val_ubo_pool_buffer;
    val_buffer_section = vulkan_rs->val_ubo_pool_buffer->allocate_section(size);
}

VulkanGraphicsBuffer::~VulkanGraphicsBuffer() {
    const VulkanRenderServer *vulkan_rs = reinterpret_cast<const VulkanRenderServer *>(RenderServer::get_singleton());

    if (val_buffer != nullptr) {
        val_buffer->release(vulkan_rs->val_instance);
        delete val_buffer;

#ifdef DEBUG
        std::cout << "Vulkan: 0x" << this << " released VulkanGraphicsBuffer::val_buffer" << std::endl;
#endif
    }

    if (val_descriptor_set != nullptr) {
        val_descriptor_set->release(vulkan_rs->val_instance);
        delete val_descriptor_set;

#ifdef DEBUG
        std::cout << "Vulkan: 0x" << this << " released VulkanGraphicsBuffer::val_descriptor_set" << std::endl;
#endif
    }
}

void VulkanGraphicsBuffer::write(void *data, size_t size) {
    const VulkanRenderServer *vulkan_rs = reinterpret_cast<const VulkanRenderServer*>(RenderServer::get_singleton());

    val_buffer->write(data, val_buffer_section, vulkan_rs->val_instance);
}
