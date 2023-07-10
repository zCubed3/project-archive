#ifndef MANTA_VKMESHBUFFER_HPP
#define MANTA_VKMESHBUFFER_HPP

#include <assets/mesh.hpp>
#include <vk_mem_alloc.h>

namespace Manta::Rendering::Vulkan {
    struct VkMeshBuffer : public MeshBuffer {
        void Create(Mesh* mesh, EngineContext* engine) override;
        void UpdateData(Mesh* mesh, EngineContext* engine) override;

    protected:
        VmaAllocation allocation;
        VkBuffer buffer;
    };
}

#endif
