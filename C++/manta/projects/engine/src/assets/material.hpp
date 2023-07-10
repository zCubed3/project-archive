#ifndef MANTA_MATERIAL_HPP
#define MANTA_MATERIAL_HPP

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

namespace Manta {
    class Shader;

    struct MaterialProperty {
        enum class Type {
            Int, Float, Vector2, Vector3, Vector4
        };

        void SetInt(int v);
        void SetFloat(float v);
        void SetVector2(glm::vec2 v);
        void SetVector3(glm::vec3 v);
        void SetVector4(glm::vec4 v);

    protected:
        union {
            int int_val;
            float float_val;
        };

        union {
            glm::vec2 v2_val;
            glm::vec3 v3_val;
            glm::vec4 v4_val;
        };
    };

    class Material {
    public:
        Material(Shader* shader);
        MaterialProperty* GetProperty(const std::string& name);

        Shader* shader;
        std::unordered_map<std::string, MaterialProperty> properties;
    };
}

#endif
