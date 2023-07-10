#ifndef SAPPHIRE_MESH_DRAW_OBJECT_H
#define SAPPHIRE_MESH_DRAW_OBJECT_H

#include <memory>

#include <engine/rendering/objects/draw_object.h>
#include <engine/rendering/buffers/object_buffer.h>

class Material;
class MeshBuffer;

// MeshDrawObject are grouped by material!
// They're sorted when being enqueued
class MeshDrawObject : public DrawObject {
    REFLECT_CLASS(MeshDrawObject, DrawObject)

public:
    std::shared_ptr<Material> material = nullptr;
    std::shared_ptr<MeshBuffer> mesh_buffer = nullptr;
    ObjectBuffer* object_buffer;

    MeshDrawObject();
    ~MeshDrawObject() override;

    void update_buffer(ObjectBufferData& data);

    void draw() override;
};


#endif
