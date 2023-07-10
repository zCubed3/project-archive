#ifndef SAPPHIRE_MATERIAL_H
#define SAPPHIRE_MATERIAL_H

#include <engine/rendering/shader.h>

class ConfigFile;
class Shader;

class Material {
public:
    std::vector<Shader::ShaderParameter> parameter_overrides;

    std::shared_ptr<Shader> shader;

    virtual ~Material() = default;

    virtual bool make_from_semd(ConfigFile *p_semd_file);

    virtual bool bind_material(ShaderPass *p_shader_pass) = 0;
};

#endif
