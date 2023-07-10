#ifndef VAL_RENDER_PASS_BUILDER_HPP
#define VAL_RENDER_PASS_BUILDER_HPP

#include <val/pipelines/val_render_pass.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace VAL {
    class ValInstance;

    // TODO: Multiple subpasses
    // TODO: Subpass dependencies
    struct ValRenderPassAttachmentInfo {
        VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;

        bool load_clear = true;
        bool store = true;

        bool stencil_load_clear = false;
        bool stencil_store = false;

        VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout final_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout ref_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    };

    struct ValRenderPassColorAttachmentInfo : ValRenderPassAttachmentInfo {};

    struct ValRenderPassDepthStencilAttachmentInfo : ValRenderPassAttachmentInfo {};

    struct ValSubpassInfo {
        std::vector<uint32_t> attachment_indices;
        bool attach_depth = true;
    };

    struct ValSubpassDependencyInfo {
        uint32_t src_subpass = VK_SUBPASS_EXTERNAL;
        uint32_t dst_subpass = VK_SUBPASS_EXTERNAL;

        VkPipelineStageFlags src_stage_flags = 0;
        VkPipelineStageFlags dst_stage_flags = 0;

        VkAccessFlags src_access_flags = 0;
        VkAccessFlags dst_access_flags = 0;

        VkDependencyFlags dependency_flags = VK_DEPENDENCY_BY_REGION_BIT;
    };

    class ValRenderPassBuilder {
        std::vector<VkAttachmentDescription> vk_attachment_descriptions;
        std::vector<VkAttachmentReference> vk_attachment_refs;
        std::vector<VkSubpassDescription> vk_subpasses;
        std::vector<VkSubpassDependency> vk_subpass_dependencies;

        // We can only have 1 depth stencil!
        // For implementation purposes it is always last!
        bool has_depth_stencil;

        void push_attachment(ValRenderPassAttachmentInfo *p_attachment_info);

    public:
        void push_color_attachment(ValRenderPassColorAttachmentInfo *p_attachment_info);
        bool push_depth_attachment(ValRenderPassDepthStencilAttachmentInfo *p_attachment_info);

        void push_subpass(ValSubpassInfo *p_subpass_info);
        void push_subpass_dependency(ValSubpassDependencyInfo *p_dependency_info);

        ValRenderPass *build(ValInstance *p_val_instance);
    };
}

#endif
