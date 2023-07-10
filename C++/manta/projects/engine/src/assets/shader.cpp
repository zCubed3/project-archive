#include "shader.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

// TODO: Make this more generic for other APIs?
#include <GL/glew.h>

#include <data/engine_context.hpp>

#include <glm/gtc/type_ptr.hpp>

namespace Manta {
    Shader* Shader::LoadCode(std::string code) {
        auto shader = new Shader();

        shader->path = "#internal";
        shader->source = code;

        shader->ProcessSource();

        return shader;
    }

    Shader* Shader::LoadFile(std::string path) {
        auto shader = new Shader();

        std::ifstream shader_file(path);

        if (!shader_file.is_open())
            return nullptr; // We couldn't find a file!

        std::stringstream str;
        str << shader_file.rdbuf();
        shader_file.close();

        shader->path = path;
        shader->source = str.str();

        shader->ProcessSource();

        return shader;
    }

    void Shader::Reload() {
        if (path != "#internal") {
            std::ifstream shader_file(path);

            if (!shader_file.is_open()) // TODO: Error on deletion
                return; // We couldn't find a file!

            std::stringstream str;
            str << shader_file.rdbuf();
            shader_file.close();

            source = str.str();
            analyzed = false;
        }
    }

    void Shader::ProcessSource() {
        auto final_src = source;
        auto version_sec = final_src.find_first_of("\n");

        if (version_sec != std::string::npos) {
            auto version_line = final_src.substr(0, version_sec);

            for (char c : version_line)
                if (c != '\n')
                    break;
            //TODO: CORRECT IT IF WE FIND ONE!

            int version_no = 0;
            sscanf(version_line.c_str(), "#version %i", &version_no);

            version = version_no;
            final_src = final_src.erase(0, version_sec);
        }

        analyzed = true;
        source = final_src;
    }

    std::string combine_keywords(const std::vector<std::string>& keywords) {
        std::stringstream keyword_buf;

        for (const auto& key : keywords)
            keyword_buf << key << " ";

        return keyword_buf.str();
    }

    bool did_compile_shader(uint32_t shader) {
        int status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

        if (status != GL_TRUE) {
            int logLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);

            char *log = new char[logLen];

            glGetShaderInfoLog(shader, logLen, nullptr, log);
            printf("Shader failed to compile with error:\n%s\n", log);

            delete[] log;
        }

        return status;
    }

    bool did_compile_program(uint32_t program) {
        int status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);

        if (status != GL_TRUE) {
            int logLen = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);

            char *log = new char[logLen];

            glGetProgramInfoLog(program, logLen, nullptr, log);
            printf("Program failed to compile with error:\n%s\n", log);

            delete[] log;
        }

        return status;
    }

    uint32_t compile_source(const std::string& source_str, int version, bool isVertex) {
        uint32_t shader = glCreateShader(isVertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);

        auto version_str = std::string("#version") + " " + std::to_string(version) + "\n";
        auto type_str = std::string("#define") + " " + (isVertex ? "VERT" : "FRAG") + "\n#define " + (isVertex ? "VERTEX" : "FRAGMENT") + "\n";

        const char *sources[3] = { version_str.c_str(), type_str.c_str(), source_str.c_str() };

        //std::cout << version_str << type_str << source_str << std::endl;

        glShaderSource(shader, 3, sources, nullptr);
        glCompileShader(shader);

        return shader;
    }

    uint32_t link_program(uint32_t vert, uint32_t frag) {
        uint32_t program = glCreateProgram();

        glAttachShader(program, vert);
        glAttachShader(program, frag);

        glLinkProgram(program);

        glDetachShader(program, vert);
        glDetachShader(program, frag);

        return program;
    }

    // Compiles a variant with the following keywords then appends it to the list of variants
    // For performance reasons variant lookup doesn't use nested vectors!
    bool Shader::Compile() {
        if (!analyzed)
            ProcessSource();

        bool passed = false;

        // TODO: Geometry shaders?
        auto vert = compile_source(source, version, true);
        bool vert_passed = did_compile_shader(vert);

        auto frag = compile_source(source, version, false);
        bool frag_passed = did_compile_shader(frag);

        if (vert_passed && frag_passed) {
            auto prog = link_program(vert, frag);
            bool prog_passed = did_compile_program(prog);

            if (!prog_passed)
                glDeleteProgram(prog);
            else {
                if (handle.has_value())
                    handle.reset();
            }

            handle = prog;
            passed = prog_passed;

            uniform_cache.clear();
        }

        glDeleteShader(vert);
        glDeleteShader(frag);

        return passed;
    }

    uint32_t Shader::Use(EngineContext* context) {
        uint32_t h = GL_INVALID_INDEX;
        if (handle.has_value())
            h = handle.value();
        else
            h = context->error_shader->handle.value();

        glUseProgram(h);
        return h;
    }

    std::optional<uint32_t> Shader::GetUniform(const std::string &name) {
        auto existing = uniform_cache.find(name);
        if (existing == uniform_cache.end()) {
            if (handle.has_value()) {
                uint32_t loc = glGetUniformLocation(handle.value(), name.c_str());

                if (loc != GL_INVALID_INDEX) {
                    uniform_cache.emplace(name, loc);
                    return loc;
                } else {
                    return {};
                }
            }
        } else
            return existing->second;

        return {};
    }

    void Shader::SetFloat(const std::string& name, float value) {
        auto uniform = GetUniform(name);

        if (uniform.has_value())
            glUniform1f(uniform.value(), value);
    }

    void Shader::SetMat4x4(const std::string& name, const glm::mat4x4 &matrix) {
        auto uniform = GetUniform(name);

        if (uniform.has_value())
            glUniformMatrix4fv(uniform.value(), 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void Shader::SetVec3(const std::string& name, const glm::vec3 &vec) {
        auto uniform = GetUniform(name);

        if (uniform.has_value())
            glUniform3fv(uniform.value(), 1, glm::value_ptr(vec));
    }

    void Shader::SetVec4(const std::string& name, const glm::vec4 &vec) {
        auto uniform = GetUniform(name);

        if (uniform.has_value())
            glUniform4fv(uniform.value(), 1, glm::value_ptr(vec));
    }

    void Shader::CreateEngineShaders(EngineContext* engine) {
        // TODO: Does this need simplification?
        engine->error_shader = LoadCode(R"(#version 330 core
        #ifdef VERT
            layout(location = 0) in vec3 _vertex;
            layout(location = 1) in vec3 _normal;

            uniform mat4 MANTA_MVP;
            uniform mat4 MANTA_M;
            uniform mat4 MANTA_M_IT;

            out vec3 world_position;
            out vec3 world_normal;

            void main() {
                gl_Position = MANTA_MVP * vec4(_vertex, 1.0);

                world_position = (MANTA_M * vec4(_vertex, 1.0)).xyz;
                world_normal = (MANTA_M_IT * vec4(_normal, 1.0)).xyz;
            }
        #endif

        #ifdef FRAG
            out vec4 col;

            in vec3 world_position;
            in vec3 world_normal;

            uniform mat4 MANTA_MVP;
            uniform vec4 MANTA_SINTIME;
            uniform vec3 MANTA_CAM_POS;

            void main() {
                vec3 view_dir = normalize(MANTA_CAM_POS - world_position);
                float f = pow(1 - max(0.0, dot(normalize(world_normal), normalize(view_dir))), 0.5) * max(0.2, abs(MANTA_SINTIME.y));
                col = vec4(vec3(1, 0, 0) * f, 1.0);
            }
        #endif
        )");

        engine->error_shader->Compile();
    }
}