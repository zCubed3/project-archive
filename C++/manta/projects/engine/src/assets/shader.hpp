#ifndef MANTA_SHADER_HPP
#define MANTA_SHADER_HPP

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

#include <glm/mat4x4.hpp>

namespace Manta {
    class EngineContext;

    class Shader {
    public:
        std::string source, path;
        std::optional<uint32_t> handle;

        static Shader* LoadCode(std::string code);
        static Shader* LoadFile(std::string path);

        void Reload();
        void ProcessSource(); // Analyzes the source, locates #version

        bool Compile();
        uint32_t Use(EngineContext* context);

        std::optional<uint32_t> GetUniform(const std::string& name);

        void SetFloat(const std::string &name, float value);
        void SetMat4x4(const std::string &name, const glm::mat4x4 &matrix);
        void SetVec3(const std::string &name, const glm::vec3 &vec);
        void SetVec4(const std::string &name, const glm::vec4 &vec);

        static void CreateEngineShaders(EngineContext* engine);

        //
        // Functions within this shader that describe how we handle it!
        //

        // TODO: Afaik Spir-V strips out custom pragmas, we need a way to describe shaders!
        // TODO: Binary / Text shader container file!

        // Depth testing functions, these are universally similar!
        enum class DepthTestFunc {
            Always, // No depth testing is done at all, this causes depth buffer artifacts!
            Less, // Draws fragment if fragment is closer than the one behind it
            Greater // Draws fragment if fragment is further than the one in front of it
        };

        // Face culling modes, between OpenGL and Vulkan, modes are flipped, don't panic though, we can fix that ourselves!
        enum class CullMode {
            Off, // Perform zero culling
            Back, // Culls a given face that points away from the viewport
            Front // Culls a given face that points toward the viewport
        };

        // Alpha blending functions
        enum class AlphaBlendFunc {
            // TODO: Add these
        };

    protected:
        int version = 330;
        bool analyzed = false;

        std::unordered_map<std::string, std::optional<uint32_t>> uniform_cache;

    };
}

#endif
