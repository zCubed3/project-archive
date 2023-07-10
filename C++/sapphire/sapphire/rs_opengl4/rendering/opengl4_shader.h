#ifndef SAPPHIRE_OPENGL4_SHADER_H
#define SAPPHIRE_OPENGL4_SHADER_H

#include <engine/rendering/shader.h>

#include <unordered_map>

class MeshDrawObject;
class OpenGL4Shader;

class OpenGL4ShaderPass : public ShaderPass {
protected:
    static uint32_t create_shader(uint32_t type, const std::vector<const char *> &sources);
    static std::string get_shader_error(uint32_t handle);
    static std::string get_program_error(uint32_t handle);

    std::string vert_code;
    std::string frag_code;

public:
    friend OpenGL4Shader;

    std::unordered_map<std::string, uint32_t> uniform_cache = {};
    uint32_t shader_handle = 0;

    uint32_t get_uniform(const std::string &var);
    uint32_t get_uniform_block(const std::string &var);

    ~OpenGL4ShaderPass() override;

    bool make_from_sesd(ConfigFile *p_sesd_file) override;
    void bind() override;

    void setup_object(MeshDrawObject *p_object);

    void create_vert_frag(OpenGL4Shader *p_shader);
};

// fyi, GLSL shaders compile through the driver!
class OpenGL4Shader : public Shader {
public:
    static OpenGL4Shader *error_shader;

    bool make_from_sesd(ConfigFile *p_sesd_file) override;

    ShaderPass *create_shader_pass() override;

    static void create_error_shader();
};

#endif
