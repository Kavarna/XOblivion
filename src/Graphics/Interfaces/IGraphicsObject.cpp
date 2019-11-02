#include "IGraphicsObject.h"

#include "../VulkanRenderer.h"


IVulkanInstanceObject::IVulkanInstanceObject()
{
    m_vulkanInstance = VulkanRenderer::Get()->getVulkanInstance();
    EVALUATE(m_vulkanInstance, nullptr, == , "Invalid instance when creating a %s object", typeid(*this).name());
}

IVulkanInstanceObject::~IVulkanInstanceObject()
{
}

auto IVulkanInstanceObject::getInstance() const -> vk::Instance
{
    return m_vulkanInstance;
}

auto IVulkanInstanceObject::setInstance(vk::Instance instance) -> void
{
    m_vulkanInstance = instance;
}

IVulkanDeviceObject::IVulkanDeviceObject()
{
    m_vulkanDevice = VulkanRenderer::Get()->getVulkanDeviceInfo();
    EVALUATE(m_vulkanDevice.m_physicalDevice, nullptr, == , "Invalid physical device when creating a %s object", typeid(*this).name());
    EVALUATE(m_vulkanDevice.m_logicalDevice, nullptr, == , "Invalid logical device when creating a %s object", typeid(*this).name());
}

IVulkanDeviceObject::~IVulkanDeviceObject()
{}

auto IVulkanDeviceObject::getDevice() const -> DeviceInfo
{
    return m_vulkanDevice;
}

auto IVulkanDeviceObject::setDevice(const DeviceInfo & instance) -> void
{
    m_vulkanDevice = instance;
}
