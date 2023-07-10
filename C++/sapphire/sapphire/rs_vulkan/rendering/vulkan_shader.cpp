#include "vulkan_shader.h"

#include <core/config/config_file.h>

#include <fstream>

#include <rs_vulkan/rendering/vulkan_render_server.h>

#include <rs_vulkan/shaders/error.spv.vert.gen.h>
#include <rs_vulkan/shaders/error.spv.frag.gen.h>

#include <rs_vulkan/shaders/depth_pass.spv.vert.gen.h>
#include <rs_vulkan/shaders/depth_pass.spv.frag.gen.h>

#include <val/val_instance.h>
#include <val/pipelines/val_shader_module.h>
#include <val/pipelines/val_pipeline_builder.h>

#ifdef DEBUG
#include <iostream>
#endif

std::vector<char> read_file(const std::string& path) {
    std::vector<char> code;
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (file.is_open()) {
        code.resize(file.tellg());
        file.seekg(0);

        file.read(code.data(), code.size());
        file.close();
    }

    return code;
}

VulkanShaderPass::~VulkanShaderPass() {
    const VulkanRenderServer *render_server = reinterpret_cast<const VulkanRenderServer *>(RenderServer::get_singleton());
    ValInstance *val_instance = render_server->val_instance;

    if (val_pipeline != nullptr) {
        val_pipeline->release(val_instance);
        delete val_pipeline;

#ifdef DEBUG
        std::cout << "Vulkan: 0x" << this << " released VulkanShaderPass::val_pipeline" << std::endl;
#endif
    }
}

bool VulkanShaderPass::make_from_sesd(ConfigFile *p_sesd_file) {
    ConfigFile::ConfigSection& section = p_sesd_file->get_section(name);

    if (section.name == name) {
        ShaderPass::make_from_sesd(p_sesd_file);

        std::string vert_path = section.try_get_string("sVertSPV");
        std::string frag_path = section.try_get_string("sFragSPV");

        if (vert_path.empty() || frag_path.empty()) {
            return false;
        }

        vert_code = read_file(vert_path);
        frag_code = read_file(frag_path);

        return true;
    }

    return false;
}

void VulkanShaderPass::bind() {
    const VulkanRenderServer *rs_instance = reinterpret_cast<const VulkanRenderServer *>(RenderServer::get_singleton());

    VkCommandBuffer active_command_buffer = rs_instance->val_active_render_target->vk_command_buffer;

    vkCmdBindPipeline(active_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, val_pipeline->vk_pipeline);
}

void VulkanShaderPass::create_vert_frag(VulkanShader *p_shader) {
    const VulkanRenderServer *render_server = reinterpret_cast<const VulkanRenderServer *>(RenderServer::get_singleton());
    ValInstance *val_instance = render_server->val_instance;

    ValPipelineBuilder builder{};

    ValShaderModuleCreateInfo vert_module_info{};
    vert_module_info.stage = ValShaderModuleCreateInfo::STAGE_VERTEX;
    vert_module_info.code = vert_code;

    ValShaderModuleCreateInfo frag_module_info{};
    frag_module_info.stage = ValShaderModuleCreateInfo::STAGE_FRAGMENT;
    frag_module_info.code = frag_code;

    ValShaderModule *vert_module = ValShaderModule::create_shader_module(&vert_module_info, val_instance);
    ValShaderModule *frag_module = ValShaderModule::create_shader_module(&frag_module_info, val_instance);

    // OpenGL culling mode
    switch (cull_mode) {
        case ShaderPass::CULL_MODE_NONE:
            builder.cull_mode = ValPipelineBuilder::CULL_MODE_OFF;
            break;

        case ShaderPass::CULL_MODE_BACK:
            builder.cull_mode = ValPipelineBuilder::CULL_MODE_BACK;
            break;

        case ShaderPass::CULL_MODE_FRONT:
            builder.cull_mode = ValPipelineBuilder::CULL_MODE_FRONT;
            break;
    }

    switch (depth_op) {
        case ShaderPass::DEPTH_OP_LESS:
            builder.depth_compare_op = ValPipelineBuilder::DEPTH_COMPARE_LESS;
            break;

        case ShaderPass::DEPTH_OP_LESS_OR_EQUAL:
            builder.depth_compare_op = ValPipelineBuilder::DEPTH_COMPARE_LESS_OR_EQUAL;
            break;

        case ShaderPass::DEPTH_OP_GREATER:
            builder.depth_compare_op = ValPipelineBuilder::DEPTH_COMPARE_GREATER;
            break;

        case ShaderPass::DEPTH_OP_GREATER_OR_EQUAL:
            builder.depth_compare_op = ValPipelineBuilder::DEPTH_COMPARE_GREATER_OR_EQUAL;
            break;

        case ShaderPass::DEPTH_OP_EQUAL:
            builder.depth_compare_op = ValPipelineBuilder::DEPTH_COMPARE_EQUAL;
            break;

        case ShaderPass::DEPTH_OP_ALWAYS:
            builder.depth_compare_op = ValPipelineBuilder::DEPTH_COMPARE_ALWAYS;
            break;
    }

    builder.depth_write = write_depth;
    builder.winding_order = ValPipelineBuilder::WINDING_ORDER_COUNTER_CLOCKWISE;

    builder.push_module(vert_module);
    builder.push_module(frag_module);

    // TODO: Temp, create passes similar to rt intents
    switch (usage) {
        default:
            builder.val_render_pass = render_server->val_window_render_pass;
            break;

        case ShaderPass::USAGE_INTENT_DEPTH_ONLY:
            builder.val_render_pass = render_server->val_shadow_render_pass;
            break;
    }

    // Our first set is the engine's "view" descriptor set
    builder.vk_descriptor_set_layouts.push_back(render_server->val_view_descriptor_info->vk_descriptor_set_layout);
    builder.vk_descriptor_set_layouts.push_back(p_shader->val_material_descriptor_set->vk_descriptor_set_layout);
    builder.vk_descriptor_set_layouts.push_back(render_server->val_object_descriptor_info->vk_descriptor_set_layout);

    val_pipeline = builder.build(render_server->val_default_vertex_input, val_instance);

    vert_module->release(val_instance);
    delete vert_module;

    frag_module->release(val_instance);
    delete frag_module;

    vert_code.clear();
    vert_code.resize(0);

    frag_code.clear();
    frag_code.resize(0);
}

VulkanShader *VulkanShader::error_shader = nullptr;
VulkanShader *VulkanShader::depth_only_shader = nullptr;

VulkanShader::~VulkanShader() {
    const VulkanRenderServer* rs_instance = reinterpret_cast<const VulkanRenderServer*>(RenderServer::get_singleton());
    ValInstance* val_instance = rs_instance->val_instance;

    if (val_material_descriptor_set != nullptr) {
        val_material_descriptor_set->release(val_instance);
        delete val_material_descriptor_set;

#ifdef DEBUG
        std::cout << "Vulkan: 0x" << this << " released VulkanShader::val_material_descriptor_set" << std::endl;
#endif
    }

    /*
    if (val_object_descriptor_set != nullptr) {
        val_object_descriptor_set->release(val_instance);
        delete val_object_descriptor_set;

#ifdef DEBUG
        std::cout << "Vulkan: 0x" << this << " released VulkanShader::val_object_descriptor_set" << std::endl;
#endif
    }
     */

    for (ShaderPass* pass: passes) {
        delete pass;
    }
}

bool VulkanShader::make_from_sesd(ConfigFile *p_sesd_file) {
    if (p_sesd_file == nullptr) {
        return false;
    }

    const VulkanRenderServer* rs_instance = reinterpret_cast<const VulkanRenderServer*>(RenderServer::get_singleton());
    ValInstance* val_instance = rs_instance->val_instance;

    Shader::make_from_sesd(p_sesd_file);

    for (ShaderParameter &parameter: parameters) {
        if (parameter.type == Shader::SHADER_PARAMETER_TEXTURE) {
            VulkanParameter vulkan_parameter{};
            vulkan_parameter.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            vulkan_parameter.stage_flags = VK_SHADER_STAGE_FRAGMENT_BIT;
            vulkan_parameter.location = parameter.location;

            vulkan_parameters.push_back(vulkan_parameter);
        }
    }

    // The second set is unique to material instances
    // The third set is unique to object instances
    ValDescriptorSetBuilder set_builder {};

    // We need to push the parameters as they are ordered
    bool has_parameters = !vulkan_parameters.empty();
    for (int i = 0; i < vulkan_parameters.size(); i++) {
        for (VulkanParameter &parameter: vulkan_parameters) {
            if (parameter.location == i) {
                set_builder.push_binding(parameter.type, 1, parameter.stage_flags);
            }
        }
    }

    // If we don't have any parameters we need to push a dummy
    if (!has_parameters) {
        set_builder.push_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    set_builder.push_pre_allocate(false);

    std::vector<ValDescriptorSetInfo *> sets = set_builder.build(val_instance);
    val_material_descriptor_set = sets[0];

    for (ShaderPass* pass: passes) {
        reinterpret_cast<VulkanShaderPass*>(pass)->create_vert_frag(this);
    }

    return true;
}

ShaderPass *VulkanShader::create_shader_pass() {
    return new VulkanShaderPass();
}

void VulkanShader::create_default_shaders() {
    std::vector<char> vert_code;
    std::vector<char> frag_code;

    const VulkanRenderServer* rs_instance = reinterpret_cast<const VulkanRenderServer*>(RenderServer::get_singleton());
    ValInstance* val_instance = rs_instance->val_instance;

    {
        vert_code.resize(sizeof(ERROR_VERT_CONTENTS));
        frag_code.resize(sizeof(ERROR_FRAG_CONTENTS));

        memcpy(vert_code.data(), ERROR_VERT_CONTENTS, sizeof(ERROR_VERT_CONTENTS));
        memcpy(frag_code.data(), ERROR_FRAG_CONTENTS, sizeof(ERROR_FRAG_CONTENTS));

        VulkanShader *shader = new VulkanShader();

        ValDescriptorSetBuilder set_builder;
        set_builder.push_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

        std::vector<ValDescriptorSetInfo *> sets = set_builder.build(val_instance);
        shader->val_material_descriptor_set = sets[0];

        VulkanShaderPass *shader_pass = new VulkanShaderPass();
        shader_pass->name = "Error";
        shader_pass->vert_code = vert_code;
        shader_pass->frag_code = frag_code;
        shader_pass->create_vert_frag(shader);

        shader->passes.push_back(shader_pass);

        VulkanShader::error_shader = shader;
    }

    {
        vert_code.resize(sizeof(DEPTH_PASS_VERT_CONTENTS));
        frag_code.resize(sizeof(DEPTH_PASS_FRAG_CONTENTS));

        memcpy(vert_code.data(), DEPTH_PASS_VERT_CONTENTS, sizeof(DEPTH_PASS_VERT_CONTENTS));
        memcpy(frag_code.data(), DEPTH_PASS_FRAG_CONTENTS, sizeof(DEPTH_PASS_FRAG_CONTENTS));

        VulkanShader *shader = new VulkanShader();

        ValDescriptorSetBuilder set_builder;
        set_builder.push_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

        std::vector<ValDescriptorSetInfo *> sets = set_builder.build(val_instance);
        shader->val_material_descriptor_set = sets[0];

        VulkanShaderPass *shader_pass = new VulkanShaderPass();
        shader_pass->name = "DepthOnly";
        shader_pass->vert_code = vert_code;
        shader_pass->frag_code = frag_code;
        shader_pass->usage = ShaderPass::USAGE_INTENT_DEPTH_ONLY;
        shader_pass->create_vert_frag(shader);

        shader->passes.push_back(shader_pass);

        VulkanShader::depth_only_shader = shader;
    }
}