#include "vulkan_render_target_data.h"

#include <engine/rendering/render_target.h>
#include <engine/rendering/texture_render_target.h>

#include <rs_vulkan/rendering/vulkan_render_server.h>
#include <rs_vulkan/rendering/vulkan_texture.h>
#include <val/render_targets/val_image_render_target.h>

#ifdef DEBUG
#include <iostream>
#endif

VulkanRenderTargetData::VulkanRenderTargetData(ValRenderTarget *p_val_render_target) {
    val_render_target = p_val_render_target;
}

VulkanRenderTargetData::~VulkanRenderTargetData() {
    RenderTargetData::~RenderTargetData();

    if (val_render_target != nullptr) {
        const VulkanRenderServer* render_server = reinterpret_cast<const VulkanRenderServer*>(RenderServer::get_singleton());
        ValInstance* val_instance = render_server->val_instance;

        val_render_target->release(val_instance);
        delete val_render_target;

#ifdef DEBUG
        std::cout << "Vulkan: 0x" << this << " released VulkanRenderTarget::val_render_target" << std::endl;
#endif
    }
}

void VulkanRenderTargetData::resize(int width, int height, RenderTarget* p_target) {
    if (p_target->get_type() == RenderTarget::TARGET_TYPE_TEXTURE) {
        const VulkanRenderServer* rs_instance = reinterpret_cast<const VulkanRenderServer*>(RenderServer::get_singleton());
        TextureRenderTarget *texture_rt = reinterpret_cast<TextureRenderTarget*>(p_target);
        ValImageRenderTarget *image_target = reinterpret_cast<ValImageRenderTarget*>(val_render_target);

        val_render_target->resize(width, height, rs_instance->val_instance);
        setup_texture_rt(texture_rt, image_target);
    }
}

void VulkanRenderTargetData::setup_texture_rt(TextureRenderTarget *p_target, ValImageRenderTarget *p_val_target) {
    if (p_target->usage_intent == TextureRenderTarget::USAGE_INTENT_SHADOW) {
        color_texture = nullptr;
    } else {
        if (color_texture == nullptr) {
            color_texture = new VulkanTexture(p_val_target->val_color_image, false);
        } else {
            color_texture->val_image = p_val_target->val_color_image;
        }
    }

    if (depth_texture == nullptr) {
        depth_texture = new VulkanTexture(p_val_target->val_depth_image, false);
    } else {
        depth_texture->val_image = p_val_target->val_depth_image;
    }
}

Texture *VulkanRenderTargetData::get_color_texture() {
    return color_texture;
}

Texture *VulkanRenderTargetData::get_depth_texture() {
    return depth_texture;
}
