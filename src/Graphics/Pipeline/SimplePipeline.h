#pragma once



#include "../Interfaces/IGraphicsPipeline.h"
#include "../Interfaces/IFrameDependent.h"

#include "Layout/TextureLayout.h"
#include "../Vertex/PositionColorVertex.h"

template <class PipelineLayoutType, class VertexType>
class SimplePipeline : 
    public IGraphicsPipeline<PipelineLayoutType, VertexType>, public IFrameDependent
{
public:
    SimplePipeline(PipelineLayoutType* pipelineLayout, vk::RenderPass pass, uint32_t subpass) : 
        m_pipelineLayout(pipelineLayout), m_renderPass(pass), m_subpass(subpass)  {};
    ~SimplePipeline() {};

public:
    // Inherited via IFrameDependent
    virtual void create(uint32_t totalFrames, uint32_t width, uint32_t height) override
    {
        this->m_depthState.setDepthTestEnable(VK_TRUE).setDepthWriteEnable(VK_TRUE)
            .setDepthBoundsTestEnable(VK_FALSE).setDepthCompareOp(vk::CompareOp::eLess)
            .setMinDepthBounds(0.0f).setMaxDepthBounds(1.0f)
            .setStencilTestEnable(VK_FALSE);
        this->m_msaaState.setAlphaToCoverageEnable(VK_FALSE).setAlphaToOneEnable(VK_FALSE)
            .setSampleShadingEnable(VK_FALSE).setMinSampleShading(0.0f)
            .setPSampleMask(nullptr).setRasterizationSamples(this->m_vulkanDevice.m_bestSampling);
        // Viewport state
        this->m_viewport.setX(0).setY(0).setWidth((float)width).setHeight((float)height).setMinDepth(0.0f).setMaxDepth(1.0f);
        this->m_scissor.setOffset({ 0,0 });
        this->m_scissor.setExtent({ width, height });
        
        this->updatePipeline(m_pipelineLayout, m_renderPass, m_subpass);
    }

    virtual void render(uint32_t frameIndex) override
    {
    }

    virtual void frameCleanup() override
    {
        this->reset();
    }

private:
    PipelineLayoutType*         m_pipelineLayout;
    vk::RenderPass              m_renderPass;
    uint32_t                    m_subpass;

};
