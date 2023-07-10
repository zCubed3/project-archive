#ifndef SAPPHIRE_RENDER_TARGET_H
#define SAPPHIRE_RENDER_TARGET_H

#include <memory>

#include <core/data/color.h>
#include <core/data/rect.h>
#include <engine/rendering/buffers/view_buffer.h>
#include <engine/scene/transform.h>

#include <glm.hpp>

class World;
class RenderTargetData;

class Light;

// Abstraction over windows and various other type of 2D attachments
// TODO: 1D / 3D render targets?
class RenderTarget {
public:
    enum TargetType {
        TARGET_TYPE_TEXTURE,
        TARGET_TYPE_WINDOW
    };

    enum ClearFlags {
        CLEAR_FLAG_DEPTH = 1,
        CLEAR_FLAG_COLOR = 2
    };

    enum ViewMatrixMode {
        VIEW_MATRIX_MODE_AUTOMATIC,
        VIEW_MATRIX_MODE_MANUAL
    };

    // TODO: Temp
    Light* light = nullptr;

    int clear_flags = ClearFlags::CLEAR_FLAG_COLOR | ClearFlags::CLEAR_FLAG_DEPTH;
    Color clear_color = Color(0.1F, 0.1F, 0.1F, 1.0F);

    // Render targets may have their own transforms
    // This is because multiple cameras may exist in the scene!
    Transform transform;
    float fov = 90.0F;
    float near_clip = 0.01F;
    float far_clip = 100.0F;

    ViewMatrixMode view_matrix_mode = ViewMatrixMode::VIEW_MATRIX_MODE_AUTOMATIC;

    ViewBufferData view_data {};
    ViewBuffer *view_buffer = nullptr;

    RenderTargetData *rt_data = nullptr;


    std::shared_ptr<World> world = nullptr;

    virtual ~RenderTarget();

    virtual Rect get_rect() = 0;
    virtual TargetType get_type() = 0;

    // Called when this attachment is first rendered to.
    // Useful for setting up matrices!
    virtual void begin_attach();
};

#endif
