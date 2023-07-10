#include "val_pipeline_builder.hpp"

#include <val/pipelines/val_descriptor_set_info.hpp>
#include <val/pipelines/val_pipeline.hpp>
#include <val/pipelines/val_render_pass.hpp>
#include <val/pipelines/val_shader_module.hpp>
#include <val/val_instance.hpp>

namespace VAL {
    void ValPipelineBuilder::push_module(ValShaderModule *p_val_shader_module) {
        // Make sure we don't have a duplicate
        bool replaced = false;
        for (VkPipelineShaderStageCreateInfo &vk_stage_info: vk_stage_infos) {
            if (vk_stage_info.stage == p_val_shader_module->vk_stage_info.stage) {
                vk_stage_info = p_val_shader_module->vk_stage_info;
                replaced = true;
            }
        }

        if (!replaced) {
            vk_stage_infos.push_back(p_val_shader_module->vk_stage_info);
        }
    }

    // TODO: Errors
    ValPipeline *ValPipelineBuilder::build(const ValVertexInputBuilder &vertex_builder, ValInstance *p_val_instance) {
        if (p_val_instance == nullptr) {
            return nullptr;
        }

        std::vector<VkVertexInputAttributeDescription> input_attributes = vertex_builder.get_input_attributes();
        VkVertexInputBindingDescription input_description = vertex_builder.get_binding_description();

        // Convert our builder inputs to vulkan equivalents

        VkCullModeFlagBits cull_flags;

        switch (cull_mode) {
            case CULL_MODE_OFF:
                cull_flags = VK_CULL_MODE_NONE;
                break;

            case CullMode::CULL_MODE_BACK:
                cull_flags = VK_CULL_MODE_BACK_BIT;
                break;

            case CullMode::CULL_MODE_FRONT:
                cull_flags = VK_CULL_MODE_FRONT_BIT;
                break;
        }

        VkPolygonMode polygon_mode;

        // TODO: Check if the device allows non-solid fill modes!
        switch (fill_mode) {
            case FillMode::FILL_MODE_FACE:
                polygon_mode = VK_POLYGON_MODE_FILL;
                break;

            case FillMode::FILL_MODE_LINE:
                polygon_mode = VK_POLYGON_MODE_LINE;
                break;

            case FillMode::FILL_MODE_POINT:
                polygon_mode = VK_POLYGON_MODE_POINT;
                break;
        }

        VkFrontFace front_face;

        // TODO: Check if the device allows non-solid fill modes!
        switch (winding_order) {
            case WindingOrder::WINDING_ORDER_COUNTER_CLOCKWISE:
                front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE;
                break;

            case WindingOrder::WINDING_ORDER_CLOCKWISE:
                front_face = VK_FRONT_FACE_CLOCKWISE;
                break;
        }

        VkCompareOp compare_op;

        switch (depth_compare_op) {
            case DEPTH_COMPARE_LESS:
                compare_op = VK_COMPARE_OP_LESS;
                break;

            case DEPTH_COMPARE_LESS_OR_EQUAL:
                compare_op = VK_COMPARE_OP_LESS_OR_EQUAL;
                break;

            case DEPTH_COMPARE_GREATER:
                compare_op = VK_COMPARE_OP_GREATER;
                break;

            case DEPTH_COMPARE_GREATER_OR_EQUAL:
                compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL;
                break;

            case DEPTH_COMPARE_EQUAL:
                compare_op = VK_COMPARE_OP_EQUAL;
                break;

            case DEPTH_COMPARE_ALWAYS:
                compare_op = VK_COMPARE_OP_ALWAYS;
                break;
        }

        // We expect the user to have pushed the modules they wish to build!

        // TODO: Dynamic state builder
        const std::vector<VkDynamicState> dynamic_states = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
        dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_create_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
        dynamic_state_create_info.pDynamicStates = dynamic_states.data();

        // TODO: Multiple binding descriptions?
        VkPipelineVertexInputStateCreateInfo vertex_input_create_info{};
        vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_create_info.vertexBindingDescriptionCount = 1;
        vertex_input_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(input_attributes.size());
        vertex_input_create_info.pVertexBindingDescriptions = &input_description;
        vertex_input_create_info.pVertexAttributeDescriptions = input_attributes.data();

        VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
        input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewport_state_create_info{};
        viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state_create_info.viewportCount = 1;
        viewport_state_create_info.scissorCount = 1;

        // TODO: Culling mode
        VkPipelineRasterizationStateCreateInfo rasterizer_create_info{};
        rasterizer_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer_create_info.depthClampEnable = clamp_depth;
        rasterizer_create_info.rasterizerDiscardEnable = allow_discard;
        rasterizer_create_info.polygonMode = polygon_mode;
        rasterizer_create_info.lineWidth = 1.0f;
        rasterizer_create_info.cullMode = cull_flags;
        rasterizer_create_info.frontFace = front_face;
        rasterizer_create_info.depthBiasEnable = VK_FALSE;
        rasterizer_create_info.depthBiasConstantFactor = 0.0f;// Optional
        rasterizer_create_info.depthBiasClamp = 0.0f;         // Optional
        rasterizer_create_info.depthBiasSlopeFactor = 0.0f;   // Optional

        // TODO: MSAA
        VkPipelineMultisampleStateCreateInfo multisampling_create_info{};
        multisampling_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling_create_info.sampleShadingEnable = VK_FALSE;
        multisampling_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling_create_info.minSampleShading = 1.0f;         // Optional
        multisampling_create_info.pSampleMask = nullptr;           // Optional
        multisampling_create_info.alphaToCoverageEnable = VK_FALSE;// Optional
        multisampling_create_info.alphaToOneEnable = VK_FALSE;     // Optional

        VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
        color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment_state.blendEnable = VK_FALSE;
        color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;// Optional
        color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;            // Optional
        color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;// Optional
        color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;            // Optional

        VkPipelineColorBlendStateCreateInfo color_blend_state{};
        color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_state.logicOpEnable = VK_FALSE;
        color_blend_state.logicOp = VK_LOGIC_OP_COPY;// Optional
        color_blend_state.attachmentCount = 1;
        color_blend_state.pAttachments = &color_blend_attachment_state;
        color_blend_state.blendConstants[0] = 0.0f;// Optional
        color_blend_state.blendConstants[1] = 0.0f;// Optional
        color_blend_state.blendConstants[2] = 0.0f;// Optional
        color_blend_state.blendConstants[3] = 0.0f;// Optional

        VkPipelineDepthStencilStateCreateInfo depth_stencil_state{};
        depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil_state.depthTestEnable = depth_test;
        depth_stencil_state.depthWriteEnable = depth_write;
        depth_stencil_state.depthCompareOp = compare_op;
        depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
        depth_stencil_state.minDepthBounds = 0.0f;// Optional
        depth_stencil_state.maxDepthBounds = 1.0f;// Optional
        depth_stencil_state.stencilTestEnable = VK_FALSE;
        depth_stencil_state.front = {};// Optional
        depth_stencil_state.back = {}; // Optional

        VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
        pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(vk_descriptor_set_layouts.size());
        pipeline_layout_create_info.pSetLayouts = vk_descriptor_set_layouts.data();
        pipeline_layout_create_info.pushConstantRangeCount = 0;   // Optional
        pipeline_layout_create_info.pPushConstantRanges = nullptr;// Optional

        VkPipelineLayout vk_pipeline_layout = nullptr;
        vkCreatePipelineLayout(p_val_instance->vk_device, &pipeline_layout_create_info, nullptr, &vk_pipeline_layout);

        VkGraphicsPipelineCreateInfo pipeline_create_info{};
        pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_create_info.stageCount = static_cast<uint32_t>(vk_stage_infos.size());
        pipeline_create_info.pStages = vk_stage_infos.data();
        pipeline_create_info.pVertexInputState = &vertex_input_create_info;
        pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
        pipeline_create_info.pViewportState = &viewport_state_create_info;
        pipeline_create_info.pRasterizationState = &rasterizer_create_info;
        pipeline_create_info.pMultisampleState = &multisampling_create_info;
        pipeline_create_info.pDepthStencilState = &depth_stencil_state;// Optional
        pipeline_create_info.pColorBlendState = &color_blend_state;
        pipeline_create_info.pDynamicState = &dynamic_state_create_info;
        pipeline_create_info.layout = vk_pipeline_layout;
        pipeline_create_info.renderPass = val_render_pass->vk_render_pass;
        pipeline_create_info.subpass = 0;
        pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;// Optional
        pipeline_create_info.basePipelineIndex = -1;             // Optional

        VkPipeline vk_pipeline;
        VkResult result = vkCreateGraphicsPipelines(p_val_instance->vk_device,
                                                    nullptr,
                                                    1,
                                                    &pipeline_create_info,
                                                    nullptr,
                                                    &vk_pipeline);

        if (result != VK_SUCCESS) {
            vkDestroyPipelineLayout(p_val_instance->vk_device, vk_pipeline_layout, nullptr);
            return nullptr;
        }

        ValPipeline *val_pipeline = new ValPipeline();

        val_pipeline->vk_pipeline = vk_pipeline;
        val_pipeline->vk_pipeline_layout = vk_pipeline_layout;

        return val_pipeline;
    }
}