#include "vulkan_texture.h"

#include <rs_vulkan/rendering/vulkan_render_server.h>

#include <val/data/val_buffer.h>
#include <val/images/val_image.h>
#include <vulkan/vulkan.h>

#if defined(IMGUI_SUPPORT)
#include <backends/imgui_impl_vulkan.h>
#endif

#ifdef DEBUG
#include <iostream>
#endif

VulkanTexture::~VulkanTexture() {
    const VulkanRenderServer *rs_instance = reinterpret_cast<const VulkanRenderServer *>(RenderServer::get_singleton());

    if (val_image != nullptr && owns_image) {
        val_image->release(rs_instance->val_instance);
        delete val_image;

#ifdef DEBUG
        std::cout << "Vulkan: 0x" << this << " released VulkanTexture::val_image" << std::endl;
#endif
    }

#if defined(IMGUI_SUPPORT)
    if (val_imgui_descriptor_set != nullptr) {
        val_imgui_descriptor_set->release(rs_instance->val_instance);
        delete val_imgui_descriptor_set;

#ifdef DEBUG
        std::cout << "Vulkan: 0x" << this << " released VulkanTexture::val_imgui_descriptor_set" << std::endl;
#endif
    }
#endif
}

VulkanTexture::VulkanTexture(ValImage *val_image, bool owns_image) {
    this->val_image = val_image;
    this->owns_image = owns_image;
}

#if defined(IMGUI_SUPPORT)
void *VulkanTexture::get_imgui_handle() {
    const VulkanRenderServer *rs_instance = reinterpret_cast<const VulkanRenderServer *>(RenderServer::get_singleton());

    if (val_imgui_descriptor_set == nullptr) {
        val_imgui_descriptor_set = rs_instance->val_imgui_descriptor_info->allocate_set(rs_instance->val_instance);
    }

    ValDescriptorSetWriteInfo write_info {};
    write_info.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_info.val_image = val_image;

    val_imgui_descriptor_set->write_binding(&write_info);
    val_imgui_descriptor_set->update_set(rs_instance->val_instance);

    return (void*)val_imgui_descriptor_set->vk_descriptor_set;
}
#endif

void VulkanTexture::create_image() {
    const VulkanRenderServer *rs_instance = reinterpret_cast<const VulkanRenderServer *>(RenderServer::get_singleton());

    ValImageCreateInfo create_info{};
    create_info.generate_mips = true; // TODO: Optional?
    create_info.extent.width = width;
    create_info.extent.height = height;
    create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    create_info.usage_flags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    switch (dimensions) {
        default:
            create_info.dimensions = ValImageCreateInfo::DIMENSIONS_2D;
            break;

        case Texture::DIMENSIONS_CUBE:
            create_info.dimensions = ValImageCreateInfo::DIMENSIONS_CUBE;
            break;

        case Texture::DIMENSIONS_3D:
            create_info.dimensions = ValImageCreateInfo::DIMENSIONS_3D;
            break;
    }

    val_image = ValImage::create(&create_info, rs_instance->val_instance);
}

void VulkanTexture::load_faces(const std::vector<CubemapFace> &faces, unsigned char *shared_bytes, int shared_width, int shared_height) {
    const VulkanRenderServer *rs_instance = reinterpret_cast<const VulkanRenderServer *>(RenderServer::get_singleton());

    if (val_image == nullptr) {
        create_image();
    }

    VkCommandBuffer vk_command_buffer = rs_instance->begin_upload(false);

    if (shared_bytes != nullptr) {
        size_t length = shared_width * shared_height * 4;

        val_image->begin_write(length, vk_command_buffer, rs_instance->val_instance);
        val_image->write_buffer_data(shared_bytes, length, 0, rs_instance->val_instance);

        ValImageCubemapFaceWriteInfo write_info {};
        write_info.row_length = shared_width;

        for (const CubemapFace& face: faces) {
            write_info.face = static_cast<ValCubemapFace>(face.index);
            write_info.offset = (face.offset_y * shared_width * channels) + (face.offset_x * channels);

            val_image->write_cubemap_face(&write_info, vk_command_buffer);
        }
    } else {
        val_image->begin_write(width * height * 4 * 6, vk_command_buffer, rs_instance->val_instance);

        size_t length = width * height * 4;
        size_t offset = 0;

        for (const CubemapFace& face: faces) {
            val_image->write_buffer_data(face.unique_bytes, length, offset, rs_instance->val_instance);
            offset += length;
        }

        val_image->write_whole(vk_command_buffer);
    }

    val_image->end_write(vk_command_buffer);

    rs_instance->end_upload(vk_command_buffer, false);

    val_image->cleanup_write(rs_instance->val_instance);
}

void VulkanTexture::load_bytes(unsigned char *bytes) {
    const VulkanRenderServer *rs_instance = reinterpret_cast<const VulkanRenderServer *>(RenderServer::get_singleton());

    if (val_image == nullptr) {
        create_image();
    }

    VkCommandBuffer vk_command_buffer = rs_instance->begin_upload(false);

    val_image->begin_write(width * height * 4, vk_command_buffer, rs_instance->val_instance);
    val_image->write_buffer_data(bytes, width * height * 4, 0, rs_instance->val_instance);

    val_image->write_whole(vk_command_buffer);

    val_image->end_write(vk_command_buffer);

    rs_instance->end_upload(vk_command_buffer, false);

    val_image->cleanup_write(rs_instance->val_instance);
}
