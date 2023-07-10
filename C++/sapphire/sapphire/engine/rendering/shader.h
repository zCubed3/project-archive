#ifndef SAPPHIRE_SHADER_H
#define SAPPHIRE_SHADER_H

#include <string>
#include <unordered_map>
#include <memory>

class ConfigFile;
class TextureAsset;
class Asset;

class ShaderPass {
public:
    enum CullMode {
        CULL_MODE_BACK,
        CULL_MODE_FRONT,
        CULL_MODE_NONE
    };

    enum DepthOp {
        DEPTH_OP_LESS,
        DEPTH_OP_LESS_OR_EQUAL,
        DEPTH_OP_GREATER,
        DEPTH_OP_GREATER_OR_EQUAL,
        DEPTH_OP_EQUAL,
        DEPTH_OP_ALWAYS
    };

    enum UsageIntent {
        USAGE_INTENT_DEFAULT,
        USAGE_INTENT_DEPTH_ONLY
    };

    bool write_depth = true;
    CullMode cull_mode = CullMode::CULL_MODE_BACK;
    DepthOp depth_op = DepthOp::DEPTH_OP_LESS;
    UsageIntent usage = USAGE_INTENT_DEFAULT;

public:
    std::string name;

    virtual ~ShaderPass() = default;

    virtual bool make_from_sesd(ConfigFile *p_sesd_file);
    virtual void bind() = 0;
};

class Shader {
    // TODO: Not use friends?
    friend class MaterialLoader;

protected:
    static std::unordered_map<std::string, std::shared_ptr<Shader>> shader_cache;

    static void release_cache();

public:
    enum ShaderParameterType {
        SHADER_PARAMETER_TEXTURE
    };

    // TODO: Better abstract material parameters?
    struct ShaderParameter {
        ShaderParameterType type;
        std::string name;
        uint32_t location;
        void* data;
        std::shared_ptr<Asset> asset_ref;
    };

    std::string name;

    std::vector<ShaderPass*> passes {};
    std::vector<ShaderParameter> parameters {};

    static std::shared_ptr<Shader> get_cached_shader(const std::string& name);
    static void cache_shader(const std::shared_ptr<Shader>& shader);

    virtual ~Shader() = default;

    virtual ShaderPass* create_shader_pass() = 0;
    virtual ShaderPass* get_pass(const std::string& name);
    virtual ShaderPass* bind_pass(const std::string& name);
    virtual bool make_from_sesd(ConfigFile *p_sesd_file);
};

#endif
