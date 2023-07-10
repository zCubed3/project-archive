#include "vkmeshbuffer.hpp"

#include <rendering/vk/vkrenderer.hpp>

#include <data/engine_context.hpp>

namespace Manta::Rendering::Vulkan {
    void VkMeshBuffer::Create(Manta::Mesh *mesh, Manta::EngineContext *engine) {
        auto allocator = reinterpret_cast<VkRenderer*>(engine->renderer2)->vma_allocator;

        VkBufferCreateInfo buffer_info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        buffer_info.size = mesh->vertices.size() * sizeof(Mesh::Vertex);
        buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        alloc_info.usage = VMA_MEMORY_USAGE_AUTO; // TODO: Should this be something else?

        vmaCreateBuffer(allocator, &buffer_info, &alloc_info, &buffer, &allocation, nullptr);

        UpdateData(mesh, engine);
    }

    void VkMeshBuffer::UpdateData(Mesh *mesh, EngineContext *engine) {
        auto allocator = reinterpret_cast<VkRenderer*>(engine->renderer2)->vma_allocator;

        void* data;
        vmaMapMemory(allocator, allocation, &data);
        memcpy(data, mesh->vertices.data(), mesh->vertices.size() * sizeof(Mesh::Vertex));
        vmaUnmapMemory(allocator, allocation);
    }
}
