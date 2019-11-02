#include "Samplers.h"

Samplers::Samplers()
{
}

Samplers::~Samplers()
{
    if (m_linearAnisotropicSampler)
    {
        m_vulkanDevice.m_logicalDevice.destroySampler(m_linearAnisotropicSampler);
        m_linearAnisotropicSampler = nullptr;
    }
}

auto Samplers::getLinearAnisotropicSampler() -> vk::Sampler
{
    if (m_vulkanDevice.m_enabledFeatures.samplerAnisotropy == VK_FALSE)
        return getLinearNonAnisotropicSampler();
    if (m_linearAnisotropicSampler)
    {
        return m_linearAnisotropicSampler;
    }
    vk::SamplerCreateInfo samplerInfo = {};
    samplerInfo.setAddressModeU(vk::SamplerAddressMode::eRepeat)
        .setAddressModeV(vk::SamplerAddressMode::eRepeat)
        .setAddressModeW(vk::SamplerAddressMode::eRepeat)
        .setAnisotropyEnable(VK_TRUE).setMaxAnisotropy(16)
        .setCompareEnable(VK_FALSE)
        .setMagFilter(vk::Filter::eLinear).setMinFilter(vk::Filter::eLinear)
        .setMaxLod(FLT_MAX).setMinLod(0.0f).setMipmapMode(vk::SamplerMipmapMode::eLinear)
        .setUnnormalizedCoordinates(VK_FALSE);
    m_linearAnisotropicSampler = m_vulkanDevice.m_logicalDevice.createSampler(samplerInfo);
    return m_linearAnisotropicSampler;
}

auto Samplers::getLinearNonAnisotropicSampler() -> vk::Sampler
{
    if (m_linearNonAnisotropicSampler)
    {
        return m_linearNonAnisotropicSampler;
    }
    vk::SamplerCreateInfo samplerInfo = {};
    samplerInfo.setAddressModeU(vk::SamplerAddressMode::eRepeat)
        .setAddressModeV(vk::SamplerAddressMode::eRepeat)
        .setAddressModeW(vk::SamplerAddressMode::eRepeat)
        .setAnisotropyEnable(VK_FALSE).setMaxAnisotropy(16)
        .setCompareEnable(VK_FALSE)
        .setMagFilter(vk::Filter::eLinear).setMinFilter(vk::Filter::eLinear)
        .setMaxLod(FLT_MAX).setMinLod(0.0f).setMipmapMode(vk::SamplerMipmapMode::eLinear)
        .setUnnormalizedCoordinates(VK_FALSE);
    m_linearNonAnisotropicSampler = m_vulkanDevice.m_logicalDevice.createSampler(samplerInfo);
    return m_linearNonAnisotropicSampler;
}
