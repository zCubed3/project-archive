#include "vulkan_render_server.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>

#include <SDL.h>
#include <SDL_vulkan.h>

#include <engine/assets/mesh_asset.h>
#include <core/data/size_tools.h>
#include <engine/rendering/buffers/view_buffer.h>
#include <engine/rendering/render_target.h>
#include <engine/rendering/texture_render_target.h>
#include <engine/rendering/window_render_target.h>
#include <engine/rendering/lighting/light.h>
#include <engine/rendering/objects/mesh_draw_object.h>
#include <engine/scene/world.h>
#include <engine/assets/texture_asset.h>

#include <rs_vulkan/rendering/vulkan_graphics_buffer.h>
#include <rs_vulkan/rendering/vulkan_material.h>
#include <rs_vulkan/rendering/vulkan_mesh_buffer.h>
#include <rs_vulkan/rendering/vulkan_render_target_data.h>
#include <rs_vulkan/rendering/vulkan_shader.h>
#include <rs_vulkan/rendering/vulkan_texture.h>

#include <val/pipelines/val_pipeline.h>
#include <val/render_targets/val_image_render_target.h>

#if defined(IMGUI_SUPPORT)
#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>
#endif

#ifdef DEBUG
#include <iostream>
#define RS_VULKAN_DEBUG
#endif

// TODO: Wait for rendering to finish
VulkanRenderServer::~VulkanRenderServer() {
#if defined(IMGUI_SUPPORT)
    if (imgui_initalized) {
        ImGui_ImplVulkan_Shutdown();
    }
#endif

#ifdef DEBUG
    std::cout << "Vulkan: Shutting down render server..." << std::endl;
#endif

    val_window_render_pass->release(val_instance);
    delete val_window_render_pass;

    val_target_render_pass->release(val_instance);
    delete val_target_render_pass;

    delete VulkanShader::error_shader;
    delete VulkanShader::depth_only_shader;

    val_view_descriptor_info->release(val_instance);
    delete val_view_descriptor_info;

    delete val_instance;
}

void VulkanRenderServer::register_rs_asset_loaders() {
}

const char *VulkanRenderServer::get_name() const {
    // TODO: Include API version?
    return "Vulkan";
}

std::string VulkanRenderServer::get_error() const {
    // TODO: Vulkan render server errors
    return "";
}

uint32_t VulkanRenderServer::get_sdl_window_flags() const {
    return SDL_WINDOW_VULKAN;
}

glm::vec3 VulkanRenderServer::get_coordinate_correction() const {
    return {1, -1, 1};
}

// TODO: Vulkan render server errors
bool VulkanRenderServer::initialize(SDL_Window *p_window) {
    if (singleton != nullptr) {
        return false;
    }

    if (p_window == nullptr) {
        return false;
    }

    window = p_window;

    // Vulkan is so fun! :(

    ValInstanceCreateInfo val_create_info{};

    val_create_info.engine_name = "Sapphire Engine";
    val_create_info.application_name = "Sapphire Application";

    // TODO: Optional SDL?
    // TODO: Allow switching between sRGB and Linear at runtime
    ValInstancePresentPreferences present_preferences{};
    present_preferences.use_vsync = true;

    val_create_info.p_present_preferences = &present_preferences;

    val_create_info.p_sdl_window = p_window;
    val_create_info.instance_extensions = ValExtension::get_sdl_instance_extensions(p_window);
    val_create_info.device_extensions = {{VK_KHR_SWAPCHAIN_EXTENSION_NAME, 0}};

#ifdef RS_VULKAN_DEBUG
    val_create_info.validation_layers = {
            {"VK_LAYER_KHRONOS_validation", 0}};
#endif

    val_create_info.vk_enabled_features.fillModeNonSolid = true;

    val_create_info.requested_queues = {
            ValQueue::QueueType::QUEUE_TYPE_GRAPHICS,
            ValQueue::QueueType::QUEUE_TYPE_PRESENT,
            ValQueue::QueueType::QUEUE_TYPE_TRANSFER,
    };

    val_instance = ValInstance::create_val_instance(&val_create_info);

    // Val creates a render target for our main window
    // The surface is created but that's it, we need to initialize the rest
    ValRenderPassBuilder window_render_pass_builder{};

    ValRenderPassColorAttachmentInfo color_info{};
    color_info.format = val_instance->present_info->vk_color_format;
    color_info.final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    color_info.ref_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    ValRenderPassDepthStencilAttachmentInfo depth_stencil_info{};
    depth_stencil_info.format = val_instance->present_info->vk_depth_format;
    depth_stencil_info.final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_info.ref_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    window_render_pass_builder.push_color_attachment(&color_info);
    window_render_pass_builder.push_depth_attachment(&depth_stencil_info);

    val_window_render_pass = window_render_pass_builder.build(val_instance);

    // TODO: Render to the window as well?
    ValRenderPassBuilder target_render_pass_builder{};

    depth_stencil_info.final_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    color_info.final_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    target_render_pass_builder.push_color_attachment(&color_info);
    target_render_pass_builder.push_depth_attachment(&depth_stencil_info);

    val_target_render_pass = target_render_pass_builder.build(val_instance);

    // Shadow render pass
    ValRenderPassBuilder shadow_render_pass_builder {};

    shadow_render_pass_builder.push_depth_attachment(&depth_stencil_info);

    val_shadow_render_pass = shadow_render_pass_builder.build(val_instance);

    val_instance->val_main_window->create_swapchain(val_window_render_pass, val_instance);

    // TODO: User defined layouts?
    ValDescriptorSetBuilder val_set_builder;

    val_set_builder.push_pre_allocate(false);
    val_set_builder.push_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    val_set_builder.push_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
    val_set_builder.push_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    val_set_builder.push_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);

    val_set_builder.push_set();

    val_set_builder.push_pre_allocate(false);
    val_set_builder.push_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

    std::vector<ValDescriptorSetInfo*> sets = val_set_builder.build(val_instance);
    val_view_descriptor_info = sets[0];
    val_object_descriptor_info = sets[1];

#if defined(IMGUI_SUPPORT)
    ValDescriptorSetBuilder val_imgui_set_builder;

    val_imgui_set_builder.push_pre_allocate(false);
    val_imgui_set_builder.push_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);

    val_imgui_descriptor_info = val_imgui_set_builder.build(val_instance)[0];
#endif

    // TODO: Allow the user to create their own vertex types
    val_default_vertex_input = {};
    val_default_vertex_input.push_attribute(ValVertexInputBuilder::ATTRIBUTE_DATA_TYPE_VEC3);
    val_default_vertex_input.push_attribute(ValVertexInputBuilder::ATTRIBUTE_DATA_TYPE_VEC3);
    val_default_vertex_input.push_attribute(ValVertexInputBuilder::ATTRIBUTE_DATA_TYPE_VEC2);

    singleton = this;

    ValBufferCreateInfo ubo_create_info {};
    ubo_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    ubo_create_info.size = SizeTools::kib_to_bytes(12);
    ubo_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    val_ubo_pool_buffer = ValBuffer::create_buffer(&ubo_create_info, val_instance);

    //
    // Placeholders
    //
    VulkanShader::create_default_shaders();

    return true;
}

bool VulkanRenderServer::present(SDL_Window *p_window) {
    val_instance->val_main_window->present_queue(val_instance);
    return true;
}

void VulkanRenderServer::on_window_resized(SDL_Window *p_window) {
    if (p_window != nullptr) {
        WindowRenderTarget *rt = reinterpret_cast<WindowRenderTarget *>(SDL_GetWindowData(p_window, "RT"));
        VulkanRenderTargetData *data = reinterpret_cast<VulkanRenderTargetData *>(rt->rt_data);

        ValWindowRenderTarget *val_rt = reinterpret_cast<ValWindowRenderTarget *>(data->val_render_target);
        val_rt->create_swapchain(val_window_render_pass, val_instance);
    }
}

bool VulkanRenderServer::begin_frame() {
    val_instance->await_frame();
    vkResetFences(val_instance->vk_device, 1, &val_instance->vk_render_fence);

    val_instance->block_await = true;

    return true;
}

bool VulkanRenderServer::end_frame() {
    val_instance->block_await = false;
    return true;
}

bool VulkanRenderServer::begin_target(RenderTarget *p_target) {
    p_target->begin_attach();
    current_target = p_target;

    // Our camera and world data is already updated by this moment
    // We just have to update the binding
    VulkanGraphicsBuffer *view_buffer = reinterpret_cast<VulkanGraphicsBuffer *>(p_target->view_buffer->buffer);
    //VulkanGraphicsBuffer* world_buffer = reinterpret_cast<VulkanGraphicsBuffer*>(p_target->view_buffer->buffer);

    ValDescriptorSetWriteInfo view_write_info{};
    view_write_info.val_buffer_section = view_buffer->val_buffer_section;
    view_write_info.val_buffer = view_buffer->val_buffer;

    if (p_target->light != nullptr) {
        ValDescriptorSetWriteInfo light_write_info{};
        light_write_info.binding_index = 2;
        light_write_info.val_buffer_section = ((VulkanGraphicsBuffer*)p_target->light->buffer)->val_buffer_section;
        light_write_info.val_buffer = ((VulkanGraphicsBuffer*)p_target->light->buffer)->val_buffer;

        ValDescriptorSetWriteInfo light_texture_write_info{};
        light_texture_write_info.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        light_texture_write_info.binding_index = 1;
        light_texture_write_info.val_image = ((VulkanTexture*)p_target->light->shadow->get_depth_texture())->val_image;

        val_view_descriptor_info->write_binding(&light_write_info);
        val_view_descriptor_info->write_binding(&light_texture_write_info);
    }

    // TODO: Temp
    if (p_target->world != nullptr) {
        std::shared_ptr<World> world = p_target->world;

        if (p_target->world->skybox != nullptr) {
            ValDescriptorSetWriteInfo env_texture_write_info{};
            env_texture_write_info.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            env_texture_write_info.binding_index = 3;
            env_texture_write_info.val_image = ((VulkanTexture*)world->skybox->texture)->val_image;
            val_view_descriptor_info->write_binding(&env_texture_write_info);
        }
    }

    val_view_descriptor_info->write_binding(&view_write_info);
    val_view_descriptor_info->update_set(val_instance);

    VulkanRenderTargetData *target_data = static_cast<VulkanRenderTargetData *>(p_target->rt_data);

    if (target_data != nullptr) {
        if (target_data->val_render_target != nullptr) {
            val_active_render_target = target_data->val_render_target;

            // TODO: Use the same render pass the target was created with
            ValRenderPass* pass = val_window_render_pass;
            if (p_target->get_type() == RenderTarget::TARGET_TYPE_TEXTURE) {
                TextureRenderTarget *texture_target = reinterpret_cast<TextureRenderTarget *>(p_target);

                if (texture_target->usage_intent == TextureRenderTarget::USAGE_INTENT_GENERAL) {
                    pass = val_target_render_pass;
                } else {
                    pass = val_shadow_render_pass;
                }
            }

            val_active_render_target->clear_color = {p_target->clear_color[0], p_target->clear_color[1], p_target->clear_color[2], p_target->clear_color[3]};
            val_active_render_target->begin_render(pass, val_instance);

            // TODO: Make a dummy shader?
            VulkanShaderPass *error_shader_pass = reinterpret_cast<VulkanShaderPass*>(VulkanShader::error_shader->passes[0]);

            vkCmdBindDescriptorSets(
                    val_active_render_target->vk_command_buffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    error_shader_pass->val_pipeline->vk_pipeline_layout,
                    0,
                    1,
                    &val_view_descriptor_info->val_descriptor_set->vk_descriptor_set,
                    0,
                    nullptr);

            Rect rect = current_target->get_rect();

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float) rect.width;
            viewport.height = (float) rect.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = {static_cast<uint32_t>(rect.width), static_cast<uint32_t>(rect.height)};

            vkCmdSetViewport(val_active_render_target->vk_command_buffer, 0, 1, &viewport);
            vkCmdSetScissor(val_active_render_target->vk_command_buffer, 0, 1, &scissor);
        }
    }

    return true;
}

bool VulkanRenderServer::end_target(RenderTarget *p_target) {
    // TODO: Render the world instead
    VkCommandBuffer active_command_buffer = val_active_render_target->vk_command_buffer;

    // TODO: Allow rendering without a world?
    if (p_target->world != nullptr) {
        for (const auto& shader_iter: p_target->world->draw_tree) {
            /*
            if (shader_iter.first == nullptr) {
                continue; // TODO: Restore error shader
            }
            */

            VulkanShaderPass *shader_pass = nullptr;

            if (shader_iter.first != nullptr) {
                shader_pass = reinterpret_cast<VulkanShaderPass *>(shader_iter.first->get_pass("Lit"));
            }

            bool fallback = shader_pass == nullptr;
            if (fallback) {
                shader_pass = reinterpret_cast<VulkanShaderPass *>(VulkanShader::error_shader->passes[0]);
            }

            shader_pass->bind();

            for (const auto& material_iter: shader_iter.second) {
                if (!fallback) {
                    material_iter.first->bind_material(shader_pass);
                }

                // Our object data should've already been updated by this moment
                // We just have to update the binding
                for (MeshDrawObject *object: material_iter.second) {
                    VulkanGraphicsBuffer *object_ubo = reinterpret_cast<VulkanGraphicsBuffer *>(object->object_buffer->buffer);

                    // If we don't have an instance of the per-object descriptor we must allocate one
                    if (object_ubo->val_descriptor_set == nullptr) {
                        object_ubo->val_descriptor_set = val_object_descriptor_info->allocate_set(val_instance);
                    }

                    ValDescriptorSetWriteInfo object_write_info{};
                    object_write_info.val_buffer_section = object_ubo->val_buffer_section;
                    object_write_info.val_buffer = object_ubo->val_buffer;

                    object_ubo->val_descriptor_set->write_binding(&object_write_info);
                    object_ubo->val_descriptor_set->update_set(val_instance);

                    vkCmdBindDescriptorSets(
                            active_command_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            shader_pass->val_pipeline->vk_pipeline_layout,
                            2,
                            1,
                            &object_ubo->val_descriptor_set->vk_descriptor_set,
                            0,
                            nullptr);

                    object->draw();
                }
            }

            // TODO: Temp, make a better way of rendering shadows
            /*
            bool use_shadow = false;
            if (p_target->get_type() == RenderTarget::TARGET_TYPE_TEXTURE) {
                TextureRenderTarget *texture_target = reinterpret_cast<TextureRenderTarget *>(p_target);

                if (texture_target->usage_intent == TextureRenderTarget::USAGE_INTENT_SHADOW) {
                    use_shadow = true;
                }
            }

            if (use_shadow) {
                shader_pass = reinterpret_cast<VulkanShaderPass *>(VulkanShader::depth_only_shader->passes[0]);
                vkCmdBindPipeline(active_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader_pass->val_pipeline->vk_pipeline);
            } else {

            }
             */
        }
    }

    VulkanRenderTargetData *target_data = static_cast<VulkanRenderTargetData *>(p_target->rt_data);

    if (target_data != nullptr) {
        if (target_data->val_render_target != nullptr) {
            target_data->val_render_target->end_render(val_instance);

            // TODO: Not immediately submit

            if (target_data->val_render_target != val_instance->val_main_window) {
                val_instance->await_frame();

                if (p_target->get_type() == RenderTarget::TARGET_TYPE_WINDOW) {
                    ValWindowRenderTarget *window_rt = reinterpret_cast<ValWindowRenderTarget *>(target_data->val_render_target);
                    window_rt->present_queue(val_instance);
                }

                vkResetFences(val_instance->vk_device, 1, &val_instance->vk_render_fence);
            }
        }
    }

    current_target = nullptr;
    val_active_render_target = nullptr;

    return true;
}

GraphicsBuffer *VulkanRenderServer::create_graphics_buffer(size_t size, GraphicsBuffer::UsageIntent usage) const {
    return new VulkanGraphicsBuffer(size, usage);
}

Shader *VulkanRenderServer::create_shader() const {
    return new VulkanShader();
}

Texture *VulkanRenderServer::create_texture() const {
    return new VulkanTexture();
}

Material *VulkanRenderServer::create_material() const {
    return new VulkanMaterial();
}

void VulkanRenderServer::populate_mesh_buffer(MeshAsset *p_mesh_asset) const {
    if (p_mesh_asset != nullptr) {
        p_mesh_asset->buffer = std::shared_ptr<MeshBuffer>(new VulkanMeshBuffer(p_mesh_asset));
    }
}

void VulkanRenderServer::populate_render_target_data(RenderTarget *p_render_target) const {
    if (p_render_target != nullptr) {
        RenderServer::populate_render_target_data(p_render_target);

        ValRenderTargetCreateInfo::RenderTargetType type;
        switch (p_render_target->get_type()) {
            default:
                type = ValRenderTargetCreateInfo::RENDER_TARGET_TYPE_IMAGE;
                break;

            case RenderTarget::TARGET_TYPE_WINDOW:
                type = ValRenderTargetCreateInfo::RENDER_TARGET_TYPE_WINDOW;
                break;
        }

        ValRenderTargetCreateInfo create_info{};

        create_info.initialize_swapchain = true;
        create_info.type = type;

        // TODO: Not always assume RENDER_TARGET_TYPE_WINDOW means we can get an SDL window?
        if (type == ValRenderTargetCreateInfo::RENDER_TARGET_TYPE_WINDOW) {
            WindowRenderTarget *sdl_target = reinterpret_cast<WindowRenderTarget *>(p_render_target);
            create_info.p_window = sdl_target->window;
            create_info.val_render_pass = val_window_render_pass;

            if (sdl_target->window == val_instance->val_main_window->sdl_window) {
                // The main window for bootstrapping is already populated
                p_render_target->rt_data = new VulkanRenderTargetData(val_instance->val_main_window);
                return;
            }
        }

        if (type == ValRenderTargetCreateInfo::RENDER_TARGET_TYPE_IMAGE) {
            TextureRenderTarget *texture_target = reinterpret_cast<TextureRenderTarget *>(p_render_target);

            Rect rect = texture_target->get_rect();

            create_info.extent.width = rect.width;
            create_info.extent.height = rect.height;

            switch (texture_target->usage_intent) {
                default:
                    create_info.val_render_pass = val_target_render_pass;
                    break;

                case TextureRenderTarget::USAGE_INTENT_SHADOW:
                    create_info.val_render_pass = val_shadow_render_pass;
                    create_info.create_color = false;
                    break;
            }
        }

        ValRenderTarget *val_target = ValRenderTarget::create_render_target(&create_info, val_instance);

        VulkanRenderTargetData *vulkan_data = new VulkanRenderTargetData(val_target);
        p_render_target->rt_data = vulkan_data;

        if (type == ValRenderTargetCreateInfo::RENDER_TARGET_TYPE_IMAGE) {
            ValImageRenderTarget *image_target = reinterpret_cast<ValImageRenderTarget*>(val_target);
            TextureRenderTarget *texture_target = reinterpret_cast<TextureRenderTarget *>(p_render_target);

            vulkan_data->setup_texture_rt(texture_target, image_target);
        }
    }
}

#if defined(IMGUI_SUPPORT)
void VulkanRenderServer::initialize_imgui(WindowRenderTarget *p_target) {
    ImGuiContext* old_context = ImGui::GetCurrentContext();

    RenderServer::initialize_imgui(p_target);

    ImGui_ImplSDL2_InitForVulkan(p_target->window);

    VkDescriptorPoolSize pool_sizes[] =
            {
                    {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                    {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                    {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                    {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                    {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                    {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = 11;
    pool_info.pPoolSizes = pool_sizes;

    VkDescriptorPool vk_imgui_descriptor_pool;
    vkCreateDescriptorPool(val_instance->vk_device, &pool_info, nullptr, &vk_imgui_descriptor_pool);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = val_instance->vk_instance;
    init_info.PhysicalDevice = val_instance->vk_physical_device;
    init_info.Device = val_instance->vk_device;
    init_info.Queue = val_instance->get_queue(ValQueue::QUEUE_TYPE_GRAPHICS).vk_queue;
    init_info.DescriptorPool = vk_imgui_descriptor_pool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info, val_window_render_pass->vk_render_pass);

    VkCommandBuffer vk_upload_buffer = begin_upload(false);

    ImGui_ImplVulkan_CreateFontsTexture(vk_upload_buffer);

    end_upload(vk_upload_buffer, false);

    ImGui_ImplVulkan_DestroyFontUploadObjects();

    imgui_initalized = true;

    ImGui::SetCurrentContext(old_context);
}

bool VulkanRenderServer::begin_imgui(WindowRenderTarget *p_target) {
    ImGui::SetCurrentContext(p_target->imgui_context);
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame(p_target->window);
    ImGui::NewFrame();

    return true;
}

bool VulkanRenderServer::end_imgui(WindowRenderTarget *p_target) {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), val_active_render_target->vk_command_buffer);

    return true;
}

void VulkanRenderServer::release_imgui(WindowRenderTarget *p_target) {
    RenderServer::release_imgui(p_target);
}
#endif

VkCommandBuffer VulkanRenderServer::begin_upload(bool staging) const {
    ValQueue queue;

    if (staging) {
        queue = val_instance->get_queue(ValQueue::QUEUE_TYPE_TRANSFER);
    } else {
        queue = val_instance->get_queue(ValQueue::QUEUE_TYPE_GRAPHICS);
    }

    VkCommandBuffer vk_command_buffer = queue.allocate_buffer(val_instance);

    VkCommandBufferBeginInfo buffer_begin_info{};
    buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(vk_command_buffer, &buffer_begin_info) != VK_SUCCESS) {
        // TODO: Failed to record buffer
        vkFreeCommandBuffers(val_instance->vk_device, queue.vk_pool, 1, &vk_command_buffer);
        return nullptr;
    }

    return vk_command_buffer;
}

void VulkanRenderServer::end_upload(VkCommandBuffer buffer, bool staging) const {
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &buffer;

    vkEndCommandBuffer(buffer);

    ValQueue queue;
    if (staging) {
        queue = val_instance->get_queue(ValQueue::QUEUE_TYPE_TRANSFER);
    } else {
        queue = val_instance->get_queue(ValQueue::QUEUE_TYPE_GRAPHICS);
    }

    // TODO: Multiple uploads at once?
    vkQueueSubmit(queue.vk_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue.vk_queue);

    vkFreeCommandBuffers(val_instance->vk_device, queue.vk_pool, 1, &buffer);
}
