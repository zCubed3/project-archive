#ifndef SAPPHIRE_GRAPHICS_BUFFER_H
#define SAPPHIRE_GRAPHICS_BUFFER_H

#include <cstdlib>

class GraphicsBuffer {
public:
    enum UsageIntent {
        USAGE_INTENT_DEFAULT,
        USAGE_INTENT_LARGE_BUFFER
    };

    virtual ~GraphicsBuffer();

    virtual void write(void* data, size_t size) = 0;
};

#endif
