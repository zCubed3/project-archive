#include "val_vertex_input_builder.hpp"

namespace VAL {
    void ValVertexInputBuilder::push_attribute(AttributeDataType type) {
        uint32_t size;
        VkFormat format;

        switch (type) {
            case AttributeDataType::Float:
                size = sizeof(float);
                format = VK_FORMAT_R32_SFLOAT;
                break;

            case AttributeDataType::FVec2:
                size = sizeof(float) * 2;
                format = VK_FORMAT_R32G32_SFLOAT;
                break;

            case AttributeDataType::FVec3:
                size = sizeof(float) * 3;
                format = VK_FORMAT_R32G32B32_SFLOAT;
                break;

            case AttributeDataType::FVec4:
                size = sizeof(float) * 4;
                format = VK_FORMAT_R32G32B32A32_SFLOAT;
                break;
        }

        VkVertexInputAttributeDescription description{};
        description.binding = 0;// TODO: Allow the user to shift the binding?
        description.location = static_cast<uint32_t>(attribute_descriptions.size());
        description.format = format;
        description.offset = total_size;

        attribute_descriptions.push_back(description);
        total_size += size;
    }

    VkVertexInputBindingDescription ValVertexInputBuilder::get_binding_description() const {
        VkVertexInputBindingDescription binding_description{};
        binding_description.binding = 0;
        binding_description.stride = total_size;
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;// TODO: Instance rate?

        return binding_description;
    }

    std::vector<VkVertexInputAttributeDescription> ValVertexInputBuilder::get_input_attributes() const {
        return attribute_descriptions;
    }
}