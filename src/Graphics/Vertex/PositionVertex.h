#pragma once


#include <Oblivion.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

class PositionVertex
{
public:
    PositionVertex(float xyzw) : position(xyzw, xyzw, xyzw, xyzw) {};
    PositionVertex(float x, float y, float z, float w) : position(x, y, z, w) {};
    PositionVertex(const glm::vec4& position) : position(position) {};


    static constexpr uint32_t getVertexSize()
    {
        return sizeof(decltype(position));
    };

    static auto getVertexInputStateCreateInfo() -> vk::PipelineVertexInputStateCreateInfo
    {
        auto attributeDescription = getAttributeDescription();
        auto bindingDescription = getBindingDescription();
        vk::PipelineVertexInputStateCreateInfo vertexState = {};
        vertexState.setVertexBindingDescriptionCount((uint32_t)bindingDescription.size()).setPVertexBindingDescriptions(bindingDescription.data())
            .setVertexAttributeDescriptionCount((uint32_t)attributeDescription.size()).setPVertexAttributeDescriptions(attributeDescription.data());
        return vertexState;
    }

    static auto getAttributeDescription()->std::vector<vk::VertexInputAttributeDescription>
    {
        static std::vector<vk::VertexInputAttributeDescription> inputAttributeDescription =
        {
                                            // Location    Binding                 Format                 offset
            vk::VertexInputAttributeDescription(0,          0,        vk::Format::eR32G32B32A32Sfloat,        0)
        };
        return inputAttributeDescription;
    }

    static auto getBindingDescription()->std::array<vk::VertexInputBindingDescription, 1>
    {
        static std::array<vk::VertexInputBindingDescription, 1> inputBindingDescription =
        {
                                            // Binding            Stride                              InputRate
            vk::VertexInputBindingDescription(0,          sizeof(PositionVertex),              vk::VertexInputRate::eVertex)
        };
        return inputBindingDescription;
    }


public:
    glm::vec4 position;

};