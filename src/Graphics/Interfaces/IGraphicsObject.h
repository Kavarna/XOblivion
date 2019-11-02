#pragma once


// #include "../VulkanRenderer.h"

#include "../Utils/VulkanObjects.h"
#include "Oblivion.h"

#include <typeinfo>


class IVulkanInstanceObject
{
public:
    IVulkanInstanceObject();
    virtual ~IVulkanInstanceObject(); // Maybe do something?
    auto		getInstance() const->vk::Instance;
    auto		setInstance(vk::Instance instance) -> void;
    
protected:
    vk::Instance				m_vulkanInstance;
};

class IVulkanDeviceObject : public IVulkanInstanceObject
{
public:
    IVulkanDeviceObject();
    virtual ~IVulkanDeviceObject(); // Maybe do something?

    auto		getDevice() const -> DeviceInfo;
    auto		setDevice(const DeviceInfo& instance) -> void;

protected:
    DeviceInfo       m_vulkanDevice;
};