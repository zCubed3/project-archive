#ifndef VAL_IMAGE_HPP
#define VAL_IMAGE_HPP

#include <val/val_releasable.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace VAL {
    struct ValSamplerCreateInfo {
        VkFilter mag_filter = VK_FILTER_LINEAR;
        VkFilter min_filter = VK_FILTER_LINEAR;

        VkSamplerAddressMode mode_u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode mode_v = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode mode_w = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        VkSamplerMipmapMode mip_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    };

    struct ValImageCreateInfo {
        enum Dimensions {
            DIMENSIONS_1D,
            DIMENSIONS_2D,
            DIMENSIONS_3D,
            DIMENSIONS_CUBE
        };

        VkExtent3D extent{1, 1, 1};
        uint32_t array_size = 1;
        // TODO: Manual mip loading?
        bool generate_mips = false;

        Dimensions dimensions = DIMENSIONS_2D;

        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
        VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
        VkImageUsageFlags usage_flags = 0;
        VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;

        bool create_sampler = true;
        ValSamplerCreateInfo sampler_info{};
    };

    enum ValCubemapFace {
        VAL_CUBEMAP_FACE_RIGHT,
        VAL_CUBEMAP_FACE_LEFT,
        VAL_CUBEMAP_FACE_UP,
        VAL_CUBEMAP_FACE_DOWN,
        VAL_CUBEMAP_FACE_FRONT,
        VAL_CUBEMAP_FACE_BACK
    };

    struct ValImageSliceInfo {
        VkOffset3D offset{0, 0, 0};
        VkExtent3D extent{1, 1, 1};
    };

    struct ValImageCubemapFaceWriteInfo {
        ValCubemapFace face;

        int offset = 0;
        int row_length = 0;

        bool manual_slice = false;
        ValImageSliceInfo slice;
    };

    class ValBuffer;

    // Wrapper around a texture
    class ValImage : public ValReleasable {
    protected:
        ValImage() = default;
        ValBuffer *val_buffer = nullptr;

        bool autogen_mips = false;

    public:
        uint32_t mip_levels = 1;
        uint32_t layer_count = 1;

        VkExtent3D vk_extent{};
        VkImage vk_image = nullptr;
        VkImageView vk_image_view = nullptr;
        VkSampler vk_sampler = nullptr;
        VmaAllocation vma_allocation = nullptr;
        VkImageAspectFlags vk_aspect_flags = 0;

        static ValImage *create(ValImageCreateInfo *p_create_info, ValInstance *p_val_instance);

        // TODO: Don't always expect RGBA?
        // TODO: Don't always expect linear

        // Writes the buffer directly to the image, is unsafe for cubemaps!
        // For cubemaps it's recommended to use the write_cubemap functions!
        void write_whole(VkCommandBuffer vk_command_buffer);
        void write_cubemap_face(ValImageCubemapFaceWriteInfo *p_write_info, VkCommandBuffer vk_command_buffer);

        void write_buffer_data(unsigned char *bytes, size_t length, size_t offset, ValInstance *p_val_instance);

        void begin_write(size_t buffer_size, VkCommandBuffer vk_command_buffer, ValInstance *p_val_instance);
        void end_write(VkCommandBuffer vk_command_buffer);

        void cleanup_write(ValInstance *p_val_instance);

        void transfer_layout(VkImageLayout old_layout, VkImageLayout new_layout, VkCommandBuffer vk_command_buffer);
        void generate_mips(VkCommandBuffer vk_command_buffer);

        void release(ValInstance *p_val_instance) override;
    };
}

#endif
