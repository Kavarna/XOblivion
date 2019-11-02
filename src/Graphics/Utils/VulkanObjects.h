#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

enum class QueueFamilyType
{ // Might add more
    eGraphics,
    ePresent
};


struct DeviceInfo
{
    vk::Device							m_logicalDevice;
    vk::PhysicalDevice					m_physicalDevice;
    vk::PhysicalDeviceFeatures          m_enabledFeatures;
    vk::SampleCountFlagBits             m_bestSampling;
    struct
    {
        uint32_t graphicsIndex;
        uint32_t presentIndex;
    }									m_families;
    struct
    {
        vk::Queue graphicsQueue;
        vk::Queue presentQueue;
    }									m_queues;

};


struct SwapchainCreateInfo
{
    vk::Extent2D						m_extent;
    vk::PresentModeKHR					m_presentMode;
    vk::SurfaceFormatKHR				m_format;
    uint32_t							m_imageCount;
};


struct SwapchainInfo
{
    std::vector<vk::Image>				m_images;
    std::vector<vk::ImageView>			m_imageViews;
};


struct SwapchainCapabilities
{
    std::vector<vk::SurfaceFormatKHR>	m_formats;
    std::vector<vk::PresentModeKHR>		m_presentModes;
    vk::SurfaceCapabilitiesKHR			m_surfaceCapabilities;
};
