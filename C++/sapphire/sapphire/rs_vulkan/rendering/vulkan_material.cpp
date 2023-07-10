#include "vulkan_material.h"

#include <engine/assets/texture_asset.h>

#include <rs_vulkan/rendering/vulkan_render_server.h>
#include <rs_vulkan/rendering/vulkan_shader.h>
#include <rs_vulkan/rendering/vulkan_texture.h>

#include <val/pipelines/val_pipeline.h>
#include <val/images/val_image.h>

#ifdef DEBUG
#include <iostream>
#endif

VulkanMaterial::~VulkanMaterial() {
    const VulkanRenderServer *rs_instance = reinterpret_cast<const VulkanRenderServer *>(RenderServer::get_singleton());

    if (val_material_descriptor_info != nullptr) {
        val_material_descriptor_info->release(rs_instance->val_instance);
        delete val_material_descriptor_info;

#ifdef DEBUG
        std::cout << "Vulkan: 0x" << this << " released VulkanMaterial::val_material_descriptor_info" << std::endl;
#endif
    }
}

bool VulkanMaterial::bind_material(ShaderPass *p_shader_pass) {
    if (p_shader_pass == nullptr) {
        return false;
    }

    const VulkanRenderServer *rs_instance = reinterpret_cast<const VulkanRenderServer *>(RenderServer::get_singleton());

    VkCommandBuffer active_command_buffer = rs_instance->val_active_render_target->vk_command_buffer;
    std::shared_ptr<VulkanShader> vk_shader = std::reinterpret_pointer_cast<VulkanShader>(shader);
    VulkanShaderPass *vk_shader_pass = reinterpret_cast<VulkanShaderPass*>(p_shader_pass);

    if (vk_shader == nullptr) {
        return false;
    }

    if (val_material_descriptor_info == nullptr && vk_shader->val_material_descriptor_set != nullptr) {
        val_material_descriptor_info = vk_shader->val_material_descriptor_set->allocate_set(rs_instance->val_instance);
    }

    // TODO: Make material parameters update only when necessary
    if (val_material_descriptor_info != nullptr) {
        if (!shader->parameters.empty()) {
            // Whenever we apply a parameter we check if it has been overriden
            for (Shader::ShaderParameter& param: shader->parameters) {
                void* data = param.data;
                std::shared_ptr<Asset> asset_ref = param.asset_ref;

                for (Shader::ShaderParameter& override_param: parameter_overrides) {
                    if (override_param.name == param.name) {
                        data = override_param.data;
                        asset_ref = override_param.asset_ref;
                        break;
                    }
                }

                if (param.type == Shader::SHADER_PARAMETER_TEXTURE) {
                    // TODO: Not reference assets and instead reference the underlying texture
                    std::shared_ptr<TextureAsset> texture = std::reinterpret_pointer_cast<TextureAsset>(asset_ref);
                    VulkanTexture* vulkan_texture = reinterpret_cast<VulkanTexture*>(texture->texture);

                    ValDescriptorSetWriteInfo write_info {};
                    write_info.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    write_info.binding_index = param.location;
                    write_info.val_image = vulkan_texture->val_image;

                    val_material_descriptor_info->write_binding(&write_info);
                }
            }

            val_material_descriptor_info->update_set(rs_instance->val_instance);

            vkCmdBindDescriptorSets(
                    active_command_buffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    vk_shader_pass->val_pipeline->vk_pipeline_layout,
                    1,
                    1,
                    &val_material_descriptor_info->vk_descriptor_set,
                    0,
                    nullptr);
        }
    }

    return p_shader_pass;
}