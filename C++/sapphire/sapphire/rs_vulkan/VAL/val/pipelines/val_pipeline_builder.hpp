#ifndef VAL_PIPELINE_BUILDER_HPP
#define VAL_PIPELINE_BUILDER_HPP

#include <cstdint>
#include <vulkan/vulkan.h>

#include <val/pipelines/val_pipeline.hpp>
#include <val/pipelines/val_vertex_input_builder.hpp>

namespace VAL {
    class ValInstance;
    class ValShaderModule;
    class ValDescriptorSetInfo;
    class ValRenderPass;

    class ValPipelineBuilder {
    protected:
        std::vector<VkPipelineShaderStageCreateInfo> vk_stage_infos;

    public:
        enum CullMode {
            CULL_MODE_OFF,
            CULL_MODE_BACK,
            CULL_MODE_FRONT
        };

        enum WindingOrder {
            WINDING_ORDER_CLOCKWISE,
            WINDING_ORDER_COUNTER_CLOCKWISE
        };

        enum FillMode {
            FILL_MODE_POINT,
            FILL_MODE_LINE,
            FILL_MODE_FACE
        };

        enum ColorMaskFlags {
            COLOR_MASK_FLAG_NONE = 0,
            COLOR_MASK_FLAG_ALL = ~0,

            COLOR_MASK_FLAG_R = 1,
            COLOR_MASK_FLAG_G = 2,
            COLOR_MASK_FLAG_B = 4,
            COLOR_MASK_FLAG_A = 8
        };

        enum ColorBlendMode {
            COLOR_BLEND_MODE_NONE,
            COLOR_BLEND_MODE_ZERO,
            COLOR_BLEND_MODE_ONE
        };

        enum ColorBlendOp {
            COLOR_BLEND_OP_NONE,
            COLOR_BLEND_OP_ADD
        };

        enum AlphaBlendMode {
            ALPHA_BLEND_MODE_NONE,
            ALPHA_BLEND_MODE_ZERO,
            ALPHA_BLEND_MODE_ONE
        };

        enum AlphaBlendOp {
            ALPHA_BLEND_OP_NONE,
            ALPHA_BLEND_OP_ADD
        };

        enum DepthCompareOp {
            DEPTH_COMPARE_LESS,
            DEPTH_COMPARE_LESS_OR_EQUAL,
            DEPTH_COMPARE_GREATER,
            DEPTH_COMPARE_GREATER_OR_EQUAL,
            DEPTH_COMPARE_EQUAL,
            DEPTH_COMPARE_ALWAYS
        };

        WindingOrder winding_order = WindingOrder::WINDING_ORDER_CLOCKWISE;
        CullMode cull_mode = CullMode::CULL_MODE_BACK;
        FillMode fill_mode = FillMode::FILL_MODE_FACE;
        bool allow_discard = false;
        bool clamp_depth = false;
        uint32_t color_mask_flags = ColorMaskFlags::COLOR_MASK_FLAG_ALL;

        bool depth_test = true;
        bool depth_write = true;
        DepthCompareOp depth_compare_op = DepthCompareOp::DEPTH_COMPARE_LESS;

        ColorBlendMode color_src_blend_mode = ColorBlendMode::COLOR_BLEND_MODE_NONE;
        ColorBlendMode color_dst_blend_mode = ColorBlendMode::COLOR_BLEND_MODE_NONE;
        ColorBlendOp color_blend_op = ColorBlendOp::COLOR_BLEND_OP_NONE;

        AlphaBlendMode alpha_src_blend_mode = AlphaBlendMode::ALPHA_BLEND_MODE_NONE;
        AlphaBlendMode alpha_dst_blend_mode = AlphaBlendMode::ALPHA_BLEND_MODE_NONE;
        AlphaBlendOp alpha_blend_op = AlphaBlendOp::ALPHA_BLEND_OP_NONE;

        ValRenderPass *val_render_pass = nullptr;
        std::vector<VkDescriptorSetLayout> vk_descriptor_set_layouts;

        void push_module(ValShaderModule *p_val_shader_module);
        ValPipeline *build(const ValVertexInputBuilder &vertex_builder, ValInstance *p_val_instance);

        // TODO: Dynamic state configuration
        // TODO: Alpha to coverage?
        // TODO: Blend constants?
        // TODO: Stenciling
        // TODO: Depth bias?
    };
}

#endif
