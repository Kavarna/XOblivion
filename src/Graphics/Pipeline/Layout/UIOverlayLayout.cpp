#include "UIOverlayLayout.h"

UIOverlayLayout::UIOverlayLayout():
    m_vertShader("Shaders/uioverlay.vert.spv"),
    m_fragShader("Shaders/uioverlay.frag.spv")
{
    // Descriptor pool
    {
        std::array<vk::DescriptorPoolSize, 1> poolSizes;
        poolSizes[0].setDescriptorCount(1).setType(vk::DescriptorType::eCombinedImageSampler);
        vk::DescriptorPoolCreateInfo poolInfo;
        poolInfo.setMaxSets(1).setPoolSizeCount((uint32_t)poolSizes.size()).setPPoolSizes(poolSizes.data());
        EVALUATE(m_descriptorPool = m_vulkanDevice.m_logicalDevice.createDescriptorPool(poolInfo),
            nullptr, == , "Unable to create a descriptor pool for UIOverlayLayout");
    }
    // Descriptor layout
    {
        vk::DescriptorSetLayoutBinding fragmentUniformBuffer;
        fragmentUniformBuffer.setBinding(0).setDescriptorCount(1)
            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setStageFlags(vk::ShaderStageFlagBits::eFragment);
        vk::DescriptorSetLayoutCreateInfo layoutInfo;
        layoutInfo.setBindingCount(1).setPBindings(&fragmentUniformBuffer);
        EVALUATE(m_descriptorLayout = m_vulkanDevice.m_logicalDevice.createDescriptorSetLayout(layoutInfo),
            nullptr, == , "Unable to create Descriptor Layout for UIOverlayLayout");
    }
    // Descriptor sets
    {
        vk::DescriptorSetAllocateInfo allocateInfo;
        allocateInfo.setDescriptorPool(m_descriptorPool).setDescriptorSetCount(1).setPSetLayouts(&m_descriptorLayout);
        m_descriptorSets = m_vulkanDevice.m_logicalDevice.allocateDescriptorSets(allocateInfo);
        EVALUATE(m_descriptorSets.size(), 0, == , "Unable to allocate descriptor sets for UIOverlayLayout");
    }
    // Pipeline layout
    {
        vk::PushConstantRange pushConstant;
        pushConstant.setOffset(0).setSize(sizeof(PushConstants)).setStageFlags(vk::ShaderStageFlagBits::eVertex);
        vk::PipelineLayoutCreateInfo layoutInfo;
        layoutInfo.setPPushConstantRanges(&pushConstant).setPushConstantRangeCount(1)
            .setSetLayoutCount(1).setPSetLayouts(&m_descriptorLayout);
        EVALUATE(m_pipelineLayout = m_vulkanDevice.m_logicalDevice.createPipelineLayout(layoutInfo),
            nullptr, == , "Unable to create Pipeline Layout for UIOverlayLayout");
    }
}

UIOverlayLayout::~UIOverlayLayout()
{
    if (m_descriptorPool)
    {
        m_vulkanDevice.m_logicalDevice.destroyDescriptorPool(m_descriptorPool);
        m_descriptorPool = nullptr;
    }
    if (m_descriptorLayout)
    {
        m_vulkanDevice.m_logicalDevice.destroyDescriptorSetLayout(m_descriptorLayout);
        m_descriptorLayout = nullptr;
    }
    if (m_pipelineLayout)
    {
        m_vulkanDevice.m_logicalDevice.destroyPipelineLayout(m_pipelineLayout);
        m_pipelineLayout = nullptr;
    }
}

auto UIOverlayLayout::setPushConstants(vk::CommandBuffer& commandBuffer, const glm::vec2& scale, const glm::vec2& translate) -> void
{
    PushConstants constants = { scale, translate };
    commandBuffer.pushConstants(m_pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants), &constants);
}

auto UIOverlayLayout::setImage(const Image* fontImage, const vk::Sampler sampler) -> void
{
    vk::DescriptorImageInfo imageInfo;
    imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
        .setImageView(fontImage->getImageView()).setSampler(sampler);

    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setDstArrayElement(0).setDstBinding(0).setDstSet(m_descriptorSets[0]).setPImageInfo(&imageInfo);
    m_vulkanDevice.m_logicalDevice.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
}

void UIOverlayLayout::bindDescriptorSets(vk::CommandBuffer& commandBuffer) const
{
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, 1, m_descriptorSets.data(), 0, nullptr);
}

vk::PipelineLayout UIOverlayLayout::getPipelineLayout() const
{
    return m_pipelineLayout;
}

std::vector<vk::PipelineShaderStageCreateInfo> UIOverlayLayout::getShadersCreateInfo() const
{
    return std::vector<vk::PipelineShaderStageCreateInfo>
    {
        m_vertShader.getShaderStageCreateInfo(),
        m_fragShader.getShaderStageCreateInfo()
    };
}
