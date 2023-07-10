#ifndef SAPPHIRE_VULKAN_SHADER_H
#define SAPPHIRE_VULKAN_SHADER_H

#include <engine/rendering/shader.h>
#include <vector>
#include <vulkan/vulkan.h>

class ValPipeline;
class ValDescriptorSetInfo;

class VulkanShader;

class VulkanShaderPass : public ShaderPass {
protected:
    std::vector<char> vert_code;
    std::vector<char> frag_code;

public:
    friend VulkanShader;

    ValPipeline* val_pipeline = nullptr;

    ~VulkanShaderPass() override;

    bool make_from_sesd(ConfigFile *p_sesd_file) override;
    void bind() override;

    void create_vert_frag(VulkanShader *p_shader);
};

class VulkanShader : public Shader {
protected:
    struct VulkanParameter {
        uint32_t location;
        VkDescriptorType type;
        uint32_t stage_flags;
    };

    std::vector<VulkanParameter> vulkan_parameters;

public:
    static VulkanShader *error_shader;
    static VulkanShader *depth_only_shader;

    ValDescriptorSetInfo * val_material_descriptor_set = nullptr;

    ~VulkanShader() override;

    bool make_from_sesd(ConfigFile *p_sesd_file) override;

    ShaderPass * create_shader_pass() override;

    static void create_default_shaders();
};


#endif
