#ifndef SAPPHIRE_DRAW_OBJECT_H
#define SAPPHIRE_DRAW_OBJECT_H

#include <engine/typing/class_registry.h>

class DrawObject {
    REFLECT_BASE_CLASS(DrawObject)

public:
    virtual ~DrawObject() = default;

    virtual void draw() = 0;
};

#endif
