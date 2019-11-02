#pragma once


#include "IPipelineLayout.h"
#include <HasMethod.h>

#if defined SAFETY_CHECKS
namespace CheckClasses
{
    CREATE_CHECK_CLASS(getVertexInputStateCreateInfo);
    CREATE_CHECK_CLASS(getBindingDescription);
    CREATE_CHECK_CLASS(getAttributeDescription);
}
#endif

template <class PipelineLayoutType, class VertexType> 
class IGraphicsPipeline : public IVulkanDeviceObject
{
#if defined SAFETY_CHECKS
    static_assert(std::is_base_of<IPipelineLayout, PipelineLayoutType>::value, 
        "Pipeline Layout passed as an argument to Pipeline MUST be of IPipelineLayout");
    static_assert(CheckClasses::has_getAttributeDescription<VertexType, std::vector<vk::VertexInputAttributeDescription>*()>::value,
        "Vertex Type MUST have a getAttributeDescription static member function");
    static_assert(CheckClasses::has_getBindingDescription<VertexType, std::vector<vk::VertexInputBindingDescription>*()>::value,
        "Vertex Type MUST have a getBindingDescription static member function");
    static_assert(CheckClasses::has_getVertexInputStateCreateInfo<VertexType, vk::PipelineVertexInputStateCreateInfo()>::value,
        "Vertex Type MUST have a getVertexInputStateCreateInfo static member function");
#endif
public:
    virtual void                updatePipeline(PipelineLayoutType* layout, vk::RenderPass renderPass, uint32_t subpass)
    {
        auto shaders = layout->getShadersCreateInfo();
        m_pipelineInfo.setLayout(layout->getPipelineLayout()).setRenderPass(renderPass).setSubpass(subpass)
            .setPStages(shaders.data()).setStageCount((uint32_t)shaders.size());

        EVALUATE(m_pipeline = m_vulkanDevice.m_logicalDevice.createGraphicsPipeline(nullptr, m_pipelineInfo), nullptr,
            == , "Couldn't create a pipeline");
    }
    virtual vk::Pipeline        getPipeline() const { return m_pipeline; };

public:
    IGraphicsPipeline()
    {
        // blend state
        m_blendAttachment.setBlendEnable(VK_FALSE).setColorWriteMask(vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
        m_blendState.setLogicOpEnable(VK_FALSE).setLogicOp(vk::LogicOp::eCopy)
            .setAttachmentCount(1).setPAttachments(&m_blendAttachment);
        
        // depth state
        m_depthState.setDepthTestEnable(VK_FALSE).setStencilTestEnable(VK_FALSE);

        // dynamic state
        m_dynamicState.setDynamicStateCount(0).setPDynamicStates(nullptr);

        // IA state
        m_inputState.setTopology(vk::PrimitiveTopology::eTriangleList).setPrimitiveRestartEnable(VK_FALSE);

        // MSAA state
        m_msaaState.setAlphaToCoverageEnable(VK_FALSE).setAlphaToOneEnable(VK_FALSE)
            .setSampleShadingEnable(VK_FALSE).setMinSampleShading(0.0f)
            .setPSampleMask(nullptr).setRasterizationSamples(vk::SampleCountFlagBits::e1);

        // Rasterizer state
        m_rasterizerState.setCullMode(vk::CullModeFlagBits::eBack).setPolygonMode(vk::PolygonMode::eFill).
            setFrontFace(vk::FrontFace::eCounterClockwise).setDepthBiasEnable(VK_FALSE).setDepthBiasClamp(0.0f).
            setDepthBiasConstantFactor(0.0f).setDepthBiasSlopeFactor(0.0f).setDepthClampEnable(VK_FALSE).
            setLineWidth(1.0f).setRasterizerDiscardEnable(VK_FALSE);

        // Tesselation state
        m_tesselationState.setPatchControlPoints(0);

        // Viewport state
        m_viewportState.setViewportCount(1).setPViewports(&m_viewport)
            .setScissorCount(1).setPScissors(&m_scissor);

        // Pipeline Info
        m_vertexInputState = VertexType::getVertexInputStateCreateInfo();
        m_pipelineInfo.setBasePipelineHandle(nullptr).setBasePipelineIndex(0)
            .setPColorBlendState(&m_blendState)
            .setPDepthStencilState(&m_depthState)
            .setPDynamicState(&m_dynamicState)
            .setPInputAssemblyState(&m_inputState)
            .setPMultisampleState(&m_msaaState)
            .setPRasterizationState(&m_rasterizerState)
            .setPTessellationState(&m_tesselationState)
            .setPViewportState(&m_viewportState)
            .setPVertexInputState(&m_vertexInputState);
    }

    virtual ~IGraphicsPipeline()
    {
        reset();
    }

    void reset()
    {
        if (m_pipeline)
        {
            m_vulkanDevice.m_logicalDevice.destroyPipeline(m_pipeline);
            m_pipeline = nullptr;
        }
    }

protected:
    vk::Pipeline                                m_pipeline;
    vk::GraphicsPipelineCreateInfo              m_pipelineInfo;

    vk::PipelineColorBlendAttachmentState       m_blendAttachment;
    vk::PipelineColorBlendStateCreateInfo       m_blendState;
    vk::PipelineDepthStencilStateCreateInfo     m_depthState;
    vk::PipelineDynamicStateCreateInfo          m_dynamicState;
    vk::PipelineInputAssemblyStateCreateInfo    m_inputState;
    vk::PipelineMultisampleStateCreateInfo      m_msaaState;
    vk::PipelineRasterizationStateCreateInfo    m_rasterizerState;
    
    vk::Viewport                                m_viewport;
    vk::Rect2D                                  m_scissor;
    vk::PipelineViewportStateCreateInfo         m_viewportState;

    vk::PipelineTessellationStateCreateInfo     m_tesselationState;
    vk::PipelineVertexInputStateCreateInfo      m_vertexInputState;

};

