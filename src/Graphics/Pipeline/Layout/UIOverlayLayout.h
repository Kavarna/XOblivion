#pragma once


#include "../../Interfaces/IPipelineLayout.h"
#include "../../Utils/Shader.h"
#include "../../Utils/Image.h"

#include "glm/vec2.hpp"

class UIOverlayLayout :
    public IPipelineLayout
{
public:
    struct PushConstants
    {
        glm::vec2 scale;
        glm::vec2 translate;
    };

public:
    UIOverlayLayout();
    ~UIOverlayLayout();


    auto                                setPushConstants(vk::CommandBuffer& commandBuffer, const glm::vec2& scale, const glm::vec2& translate) -> void;
    auto                                setImage(const Image*, const vk::Sampler) -> void;

    // Inherited via IPipelineLayout
    virtual void bindDescriptorSets(vk::CommandBuffer& commandBuffer) const override;
    virtual vk::PipelineLayout getPipelineLayout() const override;
    virtual std::vector<vk::PipelineShaderStageCreateInfo> getShadersCreateInfo() const override;

private:
    vk::DescriptorPool                  m_descriptorPool;
    vk::DescriptorSetLayout             m_descriptorLayout;
    std::vector<vk::DescriptorSet>      m_descriptorSets;

    vk::PipelineLayout                  m_pipelineLayout;

    Shader                              m_vertShader;
    Shader                              m_fragShader;

};


