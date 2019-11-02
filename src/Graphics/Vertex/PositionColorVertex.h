#pragma once


#include <Oblivion.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

class PositionColorVertex
{
public:
    PositionColorVertex() = default;
    PositionColorVertex(float x, float y, float z, float w,
                        float r, float g, float b, float a) : position(x, y, z, w), color(r, g, b, a) {};
    PositionColorVertex(const glm::vec4& position, const glm::vec4& color) : position(position), color(color) {};


    static constexpr uint32_t getVertexSize()
    {
        return sizeof(decltype(position)) + sizeof(decltype(color));
    };

    static auto getVertexInputStateCreateInfo() -> vk::PipelineVertexInputStateCreateInfo
    {
        auto attributeDescription = getAttributeDescription();
        auto bindingDescription = getBindingDescription();
        vk::PipelineVertexInputStateCreateInfo vertexState = {};
        vertexState.setVertexBindingDescriptionCount((uint32_t)bindingDescription->size()).setPVertexBindingDescriptions(bindingDescription->data())
            .setVertexAttributeDescriptionCount((uint32_t)attributeDescription->size()).setPVertexAttributeDescriptions(attributeDescription->data());
        return vertexState;
    }

    static auto getAttributeDescription()->std::vector<vk::VertexInputAttributeDescription>*
    {
        static std::vector<vk::VertexInputAttributeDescription> inputAttributeDescription =
        {
                                        // Location    Binding                 Format                               offset
            vk::VertexInputAttributeDescription(0,          0,        vk::Format::eR32G32B32A32Sfloat,               0),
            vk::VertexInputAttributeDescription(1,          0,        vk::Format::eR32G32B32A32Sfloat,        sizeof(position))
        };
        return &inputAttributeDescription;
    }

    static auto getBindingDescription()->std::vector<vk::VertexInputBindingDescription>*
    {
        static std::vector<vk::VertexInputBindingDescription> inputBindingDescription =
        {
                                        // Binding            Stride                              InputRate
            vk::VertexInputBindingDescription(0,           getVertexSize(),              vk::VertexInputRate::eVertex)
        };
        return &inputBindingDescription;
    }

    bool operator == (const PositionColorVertex& rhs) const
    {
        return position == rhs.position && color == rhs.color;
    }

public:
    glm::vec4 position;
    glm::vec4 color;

};