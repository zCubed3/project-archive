#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <core/data/string_tools.h>
#include <core/config/config_file.h>
#include <core/platforms/platform.h>

Texture::Dimensions Texture::get_dimensions() const {
    return dimensions;
}

int Texture::get_width() const {
    return width;
}

int Texture::get_height() const {
    return height;
}

int Texture::get_channels() const {
    return channels;
}

bool Texture::load_from_setd(ConfigFile *p_setd_file) {
    std::string path = p_setd_file->try_get_string("sPath", "Texture");

    if (!path.empty() && !Platform::get_singleton()->file_exists(path)) {
        return false;
    }

    std::string string_type = p_setd_file->try_get_string("sType", "Texture");

    if (StringTools::compare(string_type, "3D")) {
        dimensions = DIMENSIONS_3D;
    } else if (StringTools::compare(string_type, "CUBE")) {
        dimensions = DIMENSIONS_CUBE;
    } else {
        dimensions = DIMENSIONS_2D;
    }

    if (dimensions == DIMENSIONS_CUBE) {
        bool unique_face = path.empty();

        std::string face_names[] = {
                "Right", "Left",
                "Up", "Down",
                "Front", "Back"
        };

        std::vector<CubemapFace> faces;
        faces.resize(6);

        int shared_width = 0;
        int shared_height = 0;
        unsigned char *shared_bytes = nullptr;

        width = p_setd_file->try_get_int("iFaceWidth", "Texture");
        height = p_setd_file->try_get_int("iFaceHeight", "Texture");

        if (!unique_face) {
            shared_bytes = stbi_load(path.c_str(), &shared_width, &shared_height, &channels, 4);

            if (shared_bytes == nullptr) {
                return false;
            }
        }

        for (int i = 0; i < 6; i++) {
            ConfigFile::ConfigSection& section = p_setd_file->get_section(face_names[i]);

            if (section.name != face_names[i]) {
                continue;
            }

            CubemapFace& face = faces[i];
            face.index = i;

            if (unique_face) {
                int face_width;
                int face_height;
                int face_channels;
                std::string unique_path = section.try_get_string("sPath");

                face.unique_bytes = stbi_load(unique_path.c_str(), &face_width, &face_height, &face_channels, 4);
            } else {
                face.offset_x = section.try_get_int("iOffsetX");
                face.offset_y = section.try_get_int("iOffsetY");
            }
        }

        load_faces(faces, shared_bytes, shared_width, shared_height);

        delete[] shared_bytes;

        for (CubemapFace& face: faces) {
            delete[] face.unique_bytes;
        }
    } else {
        if (path.empty()) {
            return false;
        }

        unsigned char *bytes = stbi_load(path.c_str(), &width, &height, &channels, 4);

        if (bytes == nullptr) {
            return false;
        }

        load_bytes(bytes);

        delete[] bytes;
    }

    return true;
}
