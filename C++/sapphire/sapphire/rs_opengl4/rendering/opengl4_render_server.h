#ifndef SAPPHIRE_OPENGL4_RENDER_SERVER_H
#define SAPPHIRE_OPENGL4_RENDER_SERVER_H

#include <engine/rendering/render_server.h>
#include <engine/rendering/shader.h>

class OpenGL4RenderServer : public RenderServer {
protected:
    enum Error {
        ERR_NONE,
        ERR_SINGLETON_CLASH,
        ERR_WINDOW_NULLPTR,
        ERR_CONTEXT_NULLPTR,
        ERR_TARGET_NULLPTR
    };

    int error = Error::ERR_NONE;
    void *gl_context = nullptr;
    SDL_Window *window = nullptr;

public:
    // Cached states (useful on CPU bound systems)
    bool write_depth = true;
    ShaderPass::CullMode cull_mode = ShaderPass::CULL_MODE_BACK;
    ShaderPass::DepthOp depth_op = ShaderPass::DEPTH_OP_LESS;

    void register_rs_asset_loaders() override;

    const char* get_name() const override;
    std::string get_error() const override;
    uint32_t get_sdl_window_flags() const override;

    bool initialize(SDL_Window *p_window) override;

    bool present(SDL_Window *p_window) override;

    bool begin_frame() override;
    bool end_frame() override;

    bool begin_target(RenderTarget *p_target) override;
    bool end_target(RenderTarget *p_target) override;

    GraphicsBuffer * create_graphics_buffer(size_t size, GraphicsBuffer::UsageIntent usage) const override;
    Shader *create_shader() const override;
    Texture * create_texture() const override;
    Material *create_material() const override;

#if defined(IMGUI_SUPPORT)
    void initialize_imgui(WindowRenderTarget *p_target) override;
    bool begin_imgui(WindowRenderTarget *p_target) override;
    bool end_imgui(WindowRenderTarget *p_target) override;
#endif

    void populate_mesh_buffer(MeshAsset *p_mesh_asset) const override;
    void populate_render_target_data(RenderTarget *p_render_target) const override;
};

#endif
