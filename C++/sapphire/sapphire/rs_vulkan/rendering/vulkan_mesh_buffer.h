#ifndef SAPPHIRE_VULKAN_MESH_BUFFER_H
#define SAPPHIRE_VULKAN_MESH_BUFFER_H

#include <engine/rendering/buffers/mesh_buffer.h>
#include <vulkan/vulkan.h>
#include <vector>

class MeshAsset;

class ValBuffer;
class ValDescriptorSet;

class VulkanMeshBuffer : public MeshBuffer {
protected:
    uint32_t tri_count;
    uint32_t sub_ibo_offset = 0;

    ValBuffer *val_mbo = nullptr;
    // TODO: Temp


public:
    VulkanMeshBuffer(MeshAsset* p_mesh_asset);
    ~VulkanMeshBuffer() override;

    void draw(ObjectBuffer* p_object_buffer, std::shared_ptr<Material> p_material) override;
};


#endif
