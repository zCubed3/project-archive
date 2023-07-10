#ifndef SAPPHIRE_RENDER_SERVER_H
#define SAPPHIRE_RENDER_SERVER_H

#include <cstdint>
#include <queue>
#include <string>
#include <unordered_map>
#include <memory>

#include <engine/rendering/buffers/graphics_buffer.h>

#include <glm.hpp>

typedef struct SDL_Window SDL_Window;

class MeshAsset;
class RenderTarget;
class Shader;
class Texture;
class Material;
class DrawObject;
class MeshDrawObject;

class WindowRenderTarget;

class TextureAsset;

// Abstraction over various rendering APIs
class RenderServer {
protected:
    static RenderServer *singleton;

    RenderTarget *current_target = nullptr;

public:
    enum DebugView {
        DEBUG_VIEW_NONE,
        DEBUG_VIEW_WIREFRAME
    };

    DebugView view = DEBUG_VIEW_NONE;

    static RenderServer *get_singleton();

    virtual ~RenderServer();

    virtual void register_rs_asset_loaders() = 0;

    virtual RenderTarget *get_current_target() const;
    virtual const char *get_name() const = 0;
    virtual std::string get_error() const = 0;
    virtual uint32_t get_sdl_window_flags() const = 0;

    // Returns the correction for different coordinate systems
    virtual glm::vec3 get_coordinate_correction() const;

    // TODO: Remove SDL dependency?
    virtual bool initialize(SDL_Window *p_window) = 0;

    virtual bool present(SDL_Window *p_window) = 0;

    virtual void on_window_resized(SDL_Window *p_window);

    virtual bool begin_frame() = 0;
    virtual bool end_frame() = 0;

    // Whenever we begin rendering to a target, we call this function
    virtual bool begin_target(RenderTarget *p_target) = 0;

    // Called whenever rendering to a target is finished
    virtual bool end_target(RenderTarget *p_target) = 0;

    // Creates a graphics buffer for generic usage within the render pipeline
    virtual GraphicsBuffer *create_graphics_buffer(size_t size, GraphicsBuffer::UsageIntent usage) const = 0;

    // Creates a shader that will be setup via a SESD
    virtual Shader *create_shader() const = 0;

    // Creates a texture that will be setup via an image loader
    virtual Texture *create_texture() const = 0;

    // Creates a material that will be setup via a SEMD
    virtual Material *create_material() const = 0;

    // Whenever a mesh is loaded various structures must be created
    virtual void populate_mesh_buffer(MeshAsset *p_mesh_asset) const = 0;

    // Whenever a render target is created various structures must be created
    virtual void populate_render_target_data(RenderTarget *p_render_target) const;

#if defined(IMGUI_SUPPORT)
    virtual void initialize_imgui(WindowRenderTarget *p_target);
    virtual bool begin_imgui(WindowRenderTarget *p_target) = 0;
    virtual bool end_imgui(WindowRenderTarget *p_target) = 0;
    virtual void release_imgui(WindowRenderTarget *p_target);
#endif
};

#endif
