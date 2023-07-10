#ifndef SAPPHIRE_VULKAN_GRAPHICS_BUFFER_H
#define SAPPHIRE_VULKAN_GRAPHICS_BUFFER_H

#include <engine/rendering/buffers/graphics_buffer.h>

class ValBufferSection;
class ValBuffer;
class ValDescriptorSet;

class VulkanGraphicsBuffer : public GraphicsBuffer {
public:
    ValBufferSection *val_buffer_section = nullptr;
    ValBuffer *val_buffer = nullptr;
    ValDescriptorSet *val_descriptor_set = nullptr;

    VulkanGraphicsBuffer(size_t size, UsageIntent usage);
    ~VulkanGraphicsBuffer() override;

    void write(void *data, size_t size) override;
};


#endif
