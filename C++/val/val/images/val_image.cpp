#include "val_image.hpp"

#include <cmath>
#include <algorithm>

#include <val/val_instance.hpp>

#ifdef DEBUG
#include <iostream>
#endif

namespace VAL {
    ValImage *ValImage::create(ValImageCreateInfo *p_create_info, ValInstance *p_val_instance) {
        if (p_create_info == nullptr) {
            return nullptr;
        }

        VkImageType image_type;
        VkImageViewType view_type;
        uint32_t image_flags = 0;

        switch (p_create_info->dimensions) {
            case ValImageCreateInfo::DIMENSIONS_1D:
                image_type = VK_IMAGE_TYPE_1D;
                view_type = VK_IMAGE_VIEW_TYPE_1D;
                break;

            case ValImageCreateInfo::DIMENSIONS_2D:
                image_type = VK_IMAGE_TYPE_2D;
                view_type = VK_IMAGE_VIEW_TYPE_2D;
                break;

            case ValImageCreateInfo::DIMENSIONS_3D:
                image_type = VK_IMAGE_TYPE_3D;
                view_type = VK_IMAGE_VIEW_TYPE_3D;
                break;

            case ValImageCreateInfo::DIMENSIONS_CUBE:
                image_type = VK_IMAGE_TYPE_2D;
                image_flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
                view_type = VK_IMAGE_VIEW_TYPE_CUBE;
                break;
        }

        uint32_t layer_count = p_create_info->array_size;
        if (p_create_info->dimensions == ValImageCreateInfo::DIMENSIONS_CUBE) {
            layer_count = 6;
        }

        // Calculate the amount of mipmaps we can handle with this texture
        // TODO: Support this for 3D textures?
        uint32_t mip_levels = 1;

        if (p_create_info->generate_mips) {
            mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(p_create_info->extent.width, p_create_info->extent.height)))) + 1;
        }

        VkImageCreateInfo image_info{};
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.imageType = image_type;
        image_info.extent.width = p_create_info->extent.width;
        image_info.extent.height = p_create_info->extent.height;
        image_info.extent.depth = p_create_info->extent.depth;
        image_info.mipLevels = mip_levels;
        image_info.arrayLayers = layer_count;
        image_info.samples = p_create_info->samples;
        image_info.format = p_create_info->format;
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;         // TODO: Do we need other image tiling types?
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;// TODO: Do we need other layouts?
        image_info.usage = p_create_info->usage_flags;
        image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;// TODO: Do we need other sharing modes?
        image_info.flags = image_flags;

        VkImage vk_image = nullptr;
        if (vkCreateImage(p_val_instance->vk_device, &image_info, nullptr, &vk_image) != VK_SUCCESS) {
            return nullptr;
        }

        // TODO: Move this to vma?
        VkMemoryRequirements mem_requirements{};
        vkGetImageMemoryRequirements(p_val_instance->vk_device, vk_image, &mem_requirements);

        VmaAllocationCreateInfo vma_alloc_info{};

        VmaAllocation vma_allocation = nullptr;
        VmaAllocationInfo vma_allocation_info;
        vmaAllocateMemoryForImage(p_val_instance->vma_allocator, vk_image, &vma_alloc_info, &vma_allocation, &vma_allocation_info);

        vmaBindImageMemory(p_val_instance->vma_allocator, vma_allocation, vk_image);

        VkImageViewCreateInfo image_view_info{};
        image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

        image_view_info.image = vk_image;
        image_view_info.viewType = view_type;
        image_view_info.format = p_create_info->format;

        image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        image_view_info.subresourceRange.aspectMask = p_create_info->aspect_flags;
        image_view_info.subresourceRange.baseMipLevel = 0;
        image_view_info.subresourceRange.levelCount = mip_levels;
        image_view_info.subresourceRange.baseArrayLayer = 0;
        image_view_info.subresourceRange.layerCount = layer_count;

        VkImageView vk_image_view = nullptr;
        if (vkCreateImageView(p_val_instance->vk_device, &image_view_info, nullptr, &vk_image_view) != VK_SUCCESS) {
            vkDestroyImage(p_val_instance->vk_device, vk_image, nullptr);
            vmaFreeMemory(p_val_instance->vma_allocator, vma_allocation);

            return nullptr;
        }

        VkSampler vk_sampler = nullptr;
        if (p_create_info->create_sampler) {
            ValSamplerCreateInfo sampler_info = p_create_info->sampler_info;

            VkSamplerCreateInfo sampler_create_info{};
            sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            sampler_create_info.magFilter = sampler_info.mag_filter;
            sampler_create_info.minFilter = sampler_info.min_filter;

            sampler_create_info.addressModeU = sampler_info.mode_u;
            sampler_create_info.addressModeV = sampler_info.mode_v;
            sampler_create_info.addressModeW = sampler_info.mode_w;

            sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            sampler_create_info.unnormalizedCoordinates = VK_FALSE;
            sampler_create_info.compareEnable = VK_FALSE;
            sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;

            // TODO: Enable Aniso if found and set the max
            sampler_create_info.anisotropyEnable = VK_FALSE;
            sampler_create_info.maxAnisotropy = 1.0F;

            sampler_create_info.mipmapMode = sampler_info.mip_mode;
            sampler_create_info.mipLodBias = 0.0f;
            sampler_create_info.minLod = 0.0f;
            sampler_create_info.maxLod = static_cast<float>(mip_levels);

            if (vkCreateSampler(p_val_instance->vk_device, &sampler_create_info, nullptr, &vk_sampler) != VK_SUCCESS) {
                // TODO: Cleanup
                return nullptr;
            }
        }

        ValImage *image = new ValImage();
        image->vk_image = vk_image;
        image->vk_image_view = vk_image_view;
        image->vk_sampler = vk_sampler;
        image->vma_allocation = vma_allocation;
        image->vk_extent = p_create_info->extent;
        image->vk_aspect_flags = p_create_info->aspect_flags;
        image->autogen_mips = true;
        image->mip_levels = mip_levels;
        image->layer_count = layer_count;

#ifdef DEBUG
        std::cout << "Vulkan: 0x" << image << " created ValImage::vk_image" << std::endl;
        std::cout << "Vulkan: 0x" << image << " created ValImage::vk_image_view" << std::endl;
        std::cout << "Vulkan: 0x" << image << " created ValImage::vk_sampler" << std::endl;
        std::cout << "Vulkan: 0x" << image << " created ValImage::vma_allocation" << std::endl;
#endif

        return image;
    }

    void ValImage::write_whole(VkCommandBuffer vk_command_buffer) {
        VkBufferImageCopy copy_region{};
        copy_region.bufferOffset = 0;
        copy_region.bufferRowLength = 0;
        copy_region.bufferImageHeight = 0;

        copy_region.imageSubresource.aspectMask = vk_aspect_flags;
        copy_region.imageSubresource.mipLevel = 0;
        copy_region.imageSubresource.baseArrayLayer = 0;
        copy_region.imageSubresource.layerCount = layer_count;

        copy_region.imageOffset = {0, 0, 0};
        copy_region.imageExtent = vk_extent;

        vkCmdCopyBufferToImage(vk_command_buffer,
                               val_buffer->vk_buffer,
                               vk_image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1,
                               &copy_region);
    }

    void ValImage::write_cubemap_face(ValImageCubemapFaceWriteInfo *p_write_info, VkCommandBuffer vk_command_buffer) {
        VkBufferImageCopy copy_region{};

        copy_region.bufferOffset = p_write_info->offset;
        copy_region.bufferRowLength = p_write_info->row_length;
        copy_region.bufferImageHeight = 0;

        copy_region.imageSubresource.aspectMask = vk_aspect_flags;
        copy_region.imageSubresource.mipLevel = 0;
        copy_region.imageSubresource.baseArrayLayer = p_write_info->face;
        copy_region.imageSubresource.layerCount = 1;

        if (p_write_info->manual_slice) {
            copy_region.imageOffset = p_write_info->slice.offset;
            copy_region.imageExtent = p_write_info->slice.extent;
        } else {
            copy_region.imageOffset = {0, 0, 0};
            copy_region.imageExtent = vk_extent;
        }

        vkCmdCopyBufferToImage(vk_command_buffer,
                               val_buffer->vk_buffer,
                               vk_image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1,
                               &copy_region);
    }

    void ValImage::write_buffer_data(unsigned char *bytes, size_t length, size_t offset, ValInstance *p_val_instance) {
        if (bytes != nullptr) {
            val_buffer->write(bytes, offset, length, p_val_instance);
        }
    }

    void ValImage::begin_write(size_t buffer_size, VkCommandBuffer vk_command_buffer, ValInstance *p_val_instance) {
        ValBufferCreateInfo create_info{};
        create_info.size = buffer_size;

        create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        val_buffer = ValBuffer::create_buffer(&create_info, p_val_instance);

        transfer_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vk_command_buffer);
    }

    void ValImage::end_write(VkCommandBuffer vk_command_buffer) {
        if (autogen_mips) {
            generate_mips(vk_command_buffer);
        } else {
            transfer_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, vk_command_buffer);
        }
    }

    void ValImage::cleanup_write(ValInstance *p_val_instance) {
        if (val_buffer != nullptr) {
            val_buffer->release(p_val_instance);
            delete val_buffer;
            val_buffer = nullptr;
        }
    }

    void ValImage::transfer_layout(VkImageLayout old_layout, VkImageLayout new_layout, VkCommandBuffer vk_command_buffer) {
        VkImageMemoryBarrier barrier{};

        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = vk_image;
        barrier.subresourceRange.aspectMask = vk_aspect_flags;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mip_levels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = layer_count;

        VkPipelineStageFlags source_stage = 0;
        VkPipelineStageFlags destination_stage = 0;

        if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        }

        if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        }

        vkCmdPipelineBarrier(
                vk_command_buffer,
                source_stage,
                destination_stage,
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,
                &barrier);
    }

    void ValImage::generate_mips(VkCommandBuffer vk_command_buffer) {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = vk_image;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = layer_count;
        barrier.subresourceRange.levelCount = 1;

        int32_t mip_width = static_cast<int32_t>(vk_extent.width);
        int32_t mip_height = static_cast<int32_t>(vk_extent.height);

        for (uint32_t i = 1; i < mip_levels; i++) {
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.subresourceRange.baseMipLevel = i - 1;

            vkCmdPipelineBarrier(vk_command_buffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &barrier);

            VkImageBlit blit{};
            blit.srcOffsets[0] = {0, 0, 0};
            blit.srcOffsets[1] = {mip_width, mip_height, 1};
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = layer_count;
            blit.dstOffsets[0] = {0, 0, 0};
            blit.dstOffsets[1] = {mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1};
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = layer_count;

            vkCmdBlitImage(vk_command_buffer,
                           vk_image,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           vk_image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &blit,
                           VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(vk_command_buffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier);

            if (mip_width > 1) {
                mip_width /= 2;
            }

            if (mip_height > 1) {
                mip_height /= 2;
            }
        }

        barrier.subresourceRange.baseMipLevel = mip_levels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(vk_command_buffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &barrier);
    }

    void ValImage::release(ValInstance *p_val_instance) {
        ValReleasable::release(p_val_instance);

        if (vk_image != nullptr) {
            vkDestroyImage(p_val_instance->vk_device, vk_image, nullptr);
            vk_image = nullptr;

#ifdef DEBUG
            std::cout << "Vulkan: 0x" << this << " released ValImage::vk_image" << std::endl;
#endif
        }

        if (vk_sampler != nullptr) {
            vkDestroySampler(p_val_instance->vk_device, vk_sampler, nullptr);
            vk_sampler = nullptr;

#ifdef DEBUG
            std::cout << "Vulkan: 0x" << this << " released ValImage::vk_sampler" << std::endl;
#endif
        }

        if (vk_image_view != nullptr) {
            vkDestroyImageView(p_val_instance->vk_device, vk_image_view, nullptr);
            vk_image_view = nullptr;

#ifdef DEBUG
            std::cout << "Vulkan: 0x" << this << " released ValImage::vk_image_view" << std::endl;
#endif
        }

        if (vma_allocation != nullptr) {
            vmaFreeMemory(p_val_instance->vma_allocator, vma_allocation);
            vma_allocation = nullptr;

#ifdef DEBUG
            std::cout << "Vulkan: 0x" << this << " released ValImage::vma_allocation" << std::endl;
#endif
        }
    }
}