#ifndef VAL_VERTEX_INPUT_BUILDER_HPP
#define VAL_VERTEX_INPUT_BUILDER_HPP

#include <vector>
#include <vulkan/vulkan.h>

namespace VAL {
    // Helps the user construct pipeline vertex input structures easier
    class ValVertexInputBuilder {
    protected:
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};
        uint32_t total_size = 0;

    public:
        enum class AttributeDataType {
            // TODO: Other data types?

            // Floating point types
            Float,
            FVec2,
            FVec3,
            FVec4,

            // TODO: Integer types

            // TODO: Double types
        };

        void push_attribute(AttributeDataType type);

        VkVertexInputBindingDescription get_binding_description() const;
        std::vector<VkVertexInputAttributeDescription> get_input_attributes() const;
    };
}

#endif
