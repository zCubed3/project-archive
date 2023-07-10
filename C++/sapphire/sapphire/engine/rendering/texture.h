#ifndef SAPPHIRE_TEXTURE_H
#define SAPPHIRE_TEXTURE_H

#include <string>
#include <vector>

class ConfigFile;

class Texture {
public:
    enum Dimensions {
        DIMENSIONS_2D,
        DIMENSIONS_3D,
        DIMENSIONS_CUBE
    };

protected:
    struct CubemapFace {
        unsigned char* unique_bytes = nullptr;
        int index;
        int offset_x;
        int offset_y;
    };

    int width;
    int height;
    int channels;
    Dimensions dimensions = DIMENSIONS_2D;

public:
    virtual Dimensions get_dimensions() const;
    virtual int get_width() const;
    virtual int get_height() const;
    virtual int get_channels() const;

    virtual ~Texture() = default;

#if defined(IMGUI_SUPPORT)
    virtual void* get_imgui_handle() = 0;
#endif

    virtual void load_faces(const std::vector<CubemapFace>& faces, unsigned char* shared_bytes, int shared_width, int shared_height) = 0;
    virtual void load_bytes(unsigned char* bytes) = 0;
    virtual bool load_from_setd(ConfigFile *p_setd_file);
};

#endif
