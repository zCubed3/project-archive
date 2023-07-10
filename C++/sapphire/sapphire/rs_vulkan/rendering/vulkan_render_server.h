#ifndef SAPPHIRE_VULKAN_RENDER_SERVER_H
#define SAPPHIRE_VULKAN_RENDER_SERVER_H

#include <engine/rendering/render_server.h>

#include <val/render_targets/val_render_target.h>
#include <val/pipelines/val_render_pass_builder.h>
#include <val/pipelines/val_vertex_input_builder.h>
#include <val/pipelines/val_descriptor_set_builder.h>
#include <val/val_instance.h>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <vector>

class ValInstance;
class ValBuffer;

class VulkanRenderServer : public RenderServer {
protected:
#if defined(IMGUI_SUPPORT)
    bool imgui_initalized = false;
    VkCommandBuffer vk_imgui_command_buffer = nullptr;
#endif

public:
    SDL_Window* window;

#if defined(IMGUI_SUPPORT)
    ValDescriptorSetInfo *val_imgui_descriptor_info = nullptr;
#endif

    ValInstance* val_instance = nullptr;

    ValDescriptorSetInfo *val_view_descriptor_info = nullptr;
    ValDescriptorSetInfo *val_object_descriptor_info = nullptr;
    ValRenderPass* val_window_render_pass;
    ValRenderPass* val_target_render_pass;
    ValRenderPass* val_shadow_render_pass;

    ValVertexInputBuilder val_default_vertex_input;
    ValRenderTarget *val_active_render_target = nullptr;

    ValBuffer* val_ubo_pool_buffer;
    ValBuffer* val_ssbo_pool_buffer;

    ~VulkanRenderServer() override;

    void register_rs_asset_loaders() override;

    const char* get_name() const override;
    std::string get_error() const override;
    uint32_t get_sdl_window_flags() const override;

    glm::vec3 get_coordinate_correction() const override;

    bool initialize(SDL_Window *p_window) override;

    bool present(SDL_Window *p_window) override;

    void on_window_resized(SDL_Window *p_window) override;

    bool begin_frame() override;
    bool end_frame() override;

    bool begin_target(RenderTarget *p_target) override;
    bool end_target(RenderTarget *p_target) override;

    GraphicsBuffer *create_graphics_buffer(size_t size, GraphicsBuffer::UsageIntent usage) const override;
    Shader *create_shader() const override;
    Texture * create_texture() const override;
    Material *create_material() const override;

    void populate_mesh_buffer(MeshAsset *p_mesh_asset) const override;
    void populate_render_target_data(RenderTarget *p_render_target) const override;

#if defined(IMGUI_SUPPORT)
    void initialize_imgui(WindowRenderTarget *p_target) override;
    bool begin_imgui(WindowRenderTarget *p_target) override;
    bool end_imgui(WindowRenderTarget *p_target) override;
    void release_imgui(WindowRenderTarget *p_target) override;
#endif

    // TODO: Make this more abstract?
    VkCommandBuffer begin_upload(bool staging = true) const;
    void end_upload(VkCommandBuffer buffer, bool staging = true) const;
};

#endif
