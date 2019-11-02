#pragma once


#include "../Interfaces/IGraphicsObject.h"
#include "VulkanAllocators.h"


class Image : public IVulkanDeviceObject
{
public:

    Image(const char* path, vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags requiredMemory, vk::MemoryPropertyFlags preferredMemory);
    Image(uint32_t width, uint32_t height,
        vk::Format format, vk::ImageUsageFlags usage, vk::ImageLayout layout,
        vk::MemoryPropertyFlags requiredMemory, vk::MemoryPropertyFlags preferredMemory,
        vk::SampleCountFlagBits samples, uint32_t mipLevels = 1);
    Image(unsigned char* source,
        size_t imageSize, uint32_t width, uint32_t height,
        vk::Format format, vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags requiredMemory, vk::MemoryPropertyFlags preferredMemory,
        vk::ImageLayout layout,
        vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1, uint32_t mipLevels = 1);
    ~Image();

public:
    auto                        getImageView() const -> vk::ImageView { return m_imageView; }

private:
    auto                        createFromPath(const char* path, vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags requiredMemory, vk::MemoryPropertyFlags preferredMemory) -> void;
    auto                        createImage(uint32_t width, uint32_t height,
        vk::Format format, vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags requiredMemory, vk::MemoryPropertyFlags preferredMemory,
        vk::SampleCountFlagBits samples, uint32_t mipLevels) -> bool;
    auto                        createFromMemory(unsigned char* source,
        size_t imageSize, uint32_t width, uint32_t height,
        vk::Format format, vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags requiredMemory, vk::MemoryPropertyFlags preferredMemory,
        vk::ImageLayout layout, vk::SampleCountFlagBits samples, uint32_t mipLevels) -> void;

    auto                        createImageView(vk::ImageViewType type) -> void;

private:
    
    auto                        imageBarrier(vk::ImageSubresourceRange range, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
        uint32_t srcQueueIndex, uint32_t dstQueueIndex, vk::CommandBuffer buffer = nullptr) -> void;

private:

    vk::Image                   m_image;
    VmaAllocation               m_memory;

    vk::ImageView               m_imageView;

    vk::ImageSubresourceRange   m_subresourceRange;
    vk::ImageCreateInfo         m_imageInfo;
    vk::ImageAspectFlags        m_imageAspectFlag;

    uint32_t                    m_width;
    uint32_t                    m_height;
    uint32_t                    m_mipLevels;
};
