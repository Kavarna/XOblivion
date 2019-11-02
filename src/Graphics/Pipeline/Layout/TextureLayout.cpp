#include "TextureLayout.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

TextureLayout::TextureLayout() :
    m_vertexShader("Shaders/basic.vert.spv"),
    m_fragmentShader("Shaders/basic.frag.spv")
{
    vk::DescriptorSetLayoutBinding bindingInfoUBO, bindingInfoTexture;
    bindingInfoUBO.setBinding(0).setDescriptorCount(1)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setStageFlags(vk::ShaderStageFlagBits::eVertex);
    bindingInfoTexture.setBinding(1).setDescriptorCount(1)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setStageFlags(vk::ShaderStageFlagBits::eFragment);

    std::vector<vk::DescriptorSetLayoutBinding> bindings = { bindingInfoUBO, bindingInfoTexture };
    vk::DescriptorSetLayoutCreateInfo vertexShaderLayout;
    vertexShaderLayout.setBindingCount((uint32_t)bindings.size())
        .setPBindings(bindings.data());
    m_descriptorLayout = m_vulkanDevice.m_logicalDevice.createDescriptorSetLayout(vertexShaderLayout);
    EVALUATE(m_descriptorLayout, nullptr, == , "Couldn't create descriptor layout for TextureLayout");

    vk::PushConstantRange range;
    range.setStageFlags(vk::ShaderStageFlagBits::eFragment)
        .setSize(sizeof(uint32_t)).setOffset(0);

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setSetLayoutCount(1).setPSetLayouts(&m_descriptorLayout)
        .setPushConstantRangeCount(1).setPPushConstantRanges(&range);
    m_layout = m_vulkanDevice.m_logicalDevice.createPipelineLayout(pipelineLayoutInfo);
    EVALUATE(m_layout, nullptr, == , "Couldn't create layout for TextureLayout");

    createDescriptorPools();
    allocateDescriptorSets();
    updateDescriptorSets();
}

TextureLayout::~TextureLayout()
{
    m_vulkanDevice.m_logicalDevice.destroyBuffer(m_vertexShaderUniformBuffer.m_buffer);
    vmaFreeMemory(g_allocator, m_vertexShaderUniformBuffer.m_memory);

    if (m_descriptorPool)
    { // m_vulkanDevice.m_logicalDevice.freeDescriptorSets(m_descriptorPool, m_descriptorSets);
        m_vulkanDevice.m_logicalDevice.destroyDescriptorPool(m_descriptorPool);
        m_descriptorPool = nullptr;
    }
    if (m_descriptorLayout)
    {
        m_vulkanDevice.m_logicalDevice.destroyDescriptorSetLayout(m_descriptorLayout);
        m_descriptorLayout = nullptr;
    }
    if (m_layout)
    {
        m_vulkanDevice.m_logicalDevice.destroyPipelineLayout(m_layout);
        m_layout = nullptr;
    }
}

void TextureLayout::update() 
{
    void* data;
    vmaMapMemory(g_allocator, m_vertexShaderUniformBuffer.m_memory, &data);
    memcpy(data, &m_uniformBufferObject, sizeof(UniformBufferObject));
    vmaUnmapMemory(g_allocator, m_vertexShaderUniformBuffer.m_memory);
}

auto TextureLayout::createDescriptorPools() -> void
{
    std::array<vk::DescriptorPoolSize, 2> descriptorPoolSizes;
    descriptorPoolSizes[0].setDescriptorCount(1);
    descriptorPoolSizes[0].setType(vk::DescriptorType::eUniformBuffer);
    descriptorPoolSizes[1].setDescriptorCount(1);
    descriptorPoolSizes[1].setType(vk::DescriptorType::eCombinedImageSampler);

    vk::DescriptorPoolCreateInfo descriptorPoolInfo = {};
    descriptorPoolInfo.setMaxSets(1)
        .setPoolSizeCount((uint32_t)descriptorPoolSizes.size()).setPPoolSizes(descriptorPoolSizes.data());

    m_descriptorPool = m_vulkanDevice.m_logicalDevice.createDescriptorPool(descriptorPoolInfo);
    EVALUATE(m_descriptorPool, nullptr, == , "Couldn't create a valid descriptor pool");
}

auto TextureLayout::allocateDescriptorSets() -> void
{
    std::vector<vk::DescriptorSetLayout> layouts(1, m_descriptorLayout);
    vk::DescriptorSetAllocateInfo allocationInfo;
    allocationInfo.setDescriptorPool(m_descriptorPool);
    allocationInfo.setDescriptorSetCount(1);
    allocationInfo.setPSetLayouts(layouts.data());

    m_descriptorSet = m_vulkanDevice.m_logicalDevice.allocateDescriptorSets(allocationInfo)[0];
    EVALUATE(m_descriptorSet, nullptr, == , "Couldn't allocate descriptor sets");
}

auto TextureLayout::updateDescriptorSets() -> void
{
    m_vertexShaderUniformBuffer = BufferUtils::createBuffer(vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vk::MemoryPropertyFlagBits::eHostCached,
        &m_vulkanDevice.m_families.graphicsIndex, 1, sizeof(UniformBufferObject));

    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.setBuffer(m_vertexShaderUniformBuffer.m_buffer)
        .setOffset(0).setRange(sizeof(UniformBufferObject));

    vk::WriteDescriptorSet writeSet;
    writeSet.setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setDstArrayElement(0).setDstBinding(0).setDstSet(m_descriptorSet).setPBufferInfo(&bufferInfo);
    m_vulkanDevice.m_logicalDevice.updateDescriptorSets(1, &writeSet, 0, nullptr);
}


std::vector<vk::PipelineShaderStageCreateInfo> TextureLayout::getShadersCreateInfo() const
{
    return std::vector<vk::PipelineShaderStageCreateInfo>
    {
        m_vertexShader.getShaderStageCreateInfo(),
            m_fragmentShader.getShaderStageCreateInfo()
    };
}

auto TextureLayout::setImage(Image* image, vk::Sampler sampler) -> void
{
    if (image)
    {
        vk::DescriptorImageInfo imageInfo;
        imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setImageView(image->getImageView()).setSampler(sampler);
        vk::WriteDescriptorSet writeSet;
        writeSet.setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setDstArrayElement(0).setDstBinding(1).setDstSet(m_descriptorSet).setPImageInfo(&imageInfo);
        m_vulkanDevice.m_logicalDevice.updateDescriptorSets(1, &writeSet, 0, nullptr);
        m_hasTexture = 1;
    }
    else
    {
        m_hasTexture = 0;
    }
}

void TextureLayout::bindDescriptorSets(vk::CommandBuffer& commandBuffer) const
{
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout,
        0, 1, &m_descriptorSet, 0, nullptr);
    commandBuffer.pushConstants(m_layout, vk::ShaderStageFlagBits::eFragment,
        0, sizeof(uint32_t), &m_hasTexture);
}
