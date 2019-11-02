#include "Image.h"


#include <stb_image.h>
#include "BufferUtils.h"
#include "VulkanAllocators.h"
#include "OneTimeCommandBuffers.h"

Image::Image(const char * path, vk::ImageUsageFlags usage,
    vk::MemoryPropertyFlags requiredMemory, vk::MemoryPropertyFlags preferredMemory)
{
    createFromPath(path, usage, requiredMemory, preferredMemory);
}

Image::Image(uint32_t width, uint32_t height,
    vk::Format format, vk::ImageUsageFlags usage, vk::ImageLayout layout,
    vk::MemoryPropertyFlags requiredMemory, vk::MemoryPropertyFlags preferredMemory,
    vk::SampleCountFlagBits samples, uint32_t mipLevels)
{
    createImage(width, height, format, usage, requiredMemory, preferredMemory, samples, mipLevels);
    createImageView(vk::ImageViewType::e2D);
    imageBarrier(m_subresourceRange, vk::ImageLayout::eUndefined, layout,
        m_vulkanDevice.m_families.graphicsIndex, m_vulkanDevice.m_families.graphicsIndex);
}

Image::Image(unsigned char* source,
    size_t imageSize, uint32_t width, uint32_t height,
    vk::Format format, vk::ImageUsageFlags usage,
    vk::MemoryPropertyFlags requiredMemory, vk::MemoryPropertyFlags preferredMemory,
    vk::ImageLayout layout, vk::SampleCountFlagBits samples, uint32_t mipLevels)
{
    createFromMemory(source, imageSize,
        width, height, format, usage, requiredMemory,
        preferredMemory, layout, samples, mipLevels);
}

Image::~Image()
{
    m_vulkanDevice.m_logicalDevice.destroyImageView(m_imageView);
    vmaFreeMemory(g_allocator, m_memory);
    m_vulkanDevice.m_logicalDevice.destroyImage(m_image);
}

auto Image::createFromPath(const char * path, vk::ImageUsageFlags usage,
    vk::MemoryPropertyFlags requiredMemory, vk::MemoryPropertyFlags preferredMemory) -> void
{
    int width, height, texChannels;
    stbi_uc* result;
    result = stbi_load(path, &width, &height, &texChannels, STBI_rgb_alpha);
    EVALUATE(result, nullptr, == , "Couldn't load texture %s", path);

    uint32_t mipLevels = (uint32_t)std::floor(std::log2(std::max(width, height))) + 1;

    size_t imageSize = width * height * 4;
    m_width = width;
    m_height = height;
    m_mipLevels = mipLevels;

    createFromMemory(result, imageSize, width, height, vk::Format::eR8G8B8A8Unorm,
        usage, requiredMemory, preferredMemory, vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::SampleCountFlagBits::e1, mipLevels);

    stbi_image_free(result);
}

auto Image::createImage(uint32_t width, uint32_t height,
    vk::Format format, vk::ImageUsageFlags usage,
    vk::MemoryPropertyFlags requiredMemory, vk::MemoryPropertyFlags preferredMemory,
    vk::SampleCountFlagBits samples, uint32_t mipLevels) -> bool
{
    if (format == vk::Format::eD32Sfloat)
    {
        m_imageAspectFlag |= vk::ImageAspectFlagBits::eDepth;
    }
    else if (format == vk::Format::eD24UnormS8Uint)
    {
        m_imageAspectFlag |= vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
    }
    if ((usage & vk::ImageUsageFlagBits::eColorAttachment) ||
        (usage & vk::ImageUsageFlagBits::eSampled))
    {
        m_imageAspectFlag |= vk::ImageAspectFlagBits::eColor;
    }

    if (mipLevels > 1)
        usage |= vk::ImageUsageFlagBits::eTransferSrc;

    m_imageInfo.setArrayLayers(1).setMipLevels(mipLevels)
        .setFormat(format)
        .setImageType(vk::ImageType::e2D).setInitialLayout(vk::ImageLayout::eUndefined)
        .setSamples(samples).setTiling(vk::ImageTiling::eOptimal)
        .setExtent(vk::Extent3D(width, height, 1))
        .setUsage(usage)
        .setPQueueFamilyIndices(&m_vulkanDevice.m_families.graphicsIndex).setQueueFamilyIndexCount(1)
        .setSharingMode(vk::SharingMode::eExclusive);

    VmaAllocationCreateInfo allocationInfo = {};
    allocationInfo.requiredFlags = (VkMemoryPropertyFlags)requiredMemory;
    allocationInfo.preferredFlags = (VkMemoryPropertyFlags)preferredMemory;

    VkResult res = vmaCreateImage(g_allocator, (VkImageCreateInfo*)&m_imageInfo, &allocationInfo,
        (VkImage*)&m_image, &m_memory, nullptr);
    if (res != VK_SUCCESS)
        return false;

    m_subresourceRange.setBaseMipLevel(0).setLevelCount(mipLevels)
        .setBaseArrayLayer(0).setLayerCount(1)
        .setAspectMask(m_imageAspectFlag);
    m_width = width;
    m_height = height;
    m_mipLevels = mipLevels;
    return true;
}

auto Image::createFromMemory(unsigned char* source,
    size_t imageSize, uint32_t width, uint32_t height,
    vk::Format format, vk::ImageUsageFlags usage,
    vk::MemoryPropertyFlags requiredMemory, vk::MemoryPropertyFlags preferredMemory,
    vk::ImageLayout layout, vk::SampleCountFlagBits samples, uint32_t mipLevels) -> void
{
    usage |= vk::ImageUsageFlagBits::eTransferDst; // We will copy into this image

    BufferUtils::Buffer temporaryBuffer;
    temporaryBuffer = BufferUtils::createBuffer(vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible, {},
        &m_vulkanDevice.m_families.graphicsIndex, 1, imageSize);


    void* pData;
    vmaMapMemory(g_allocator, temporaryBuffer.m_memory, &pData);
    memcpy(pData, source, imageSize);
    vmaUnmapMemory(g_allocator, temporaryBuffer.m_memory);


    createImage(width, height, format, usage,
        requiredMemory, preferredMemory, samples, mipLevels);

    // Make image a suitable transfer dst
    vk::ImageSubresourceRange range;
    range = m_subresourceRange;
    imageBarrier(range, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
        m_vulkanDevice.m_families.graphicsIndex, m_vulkanDevice.m_families.graphicsIndex);

    // Copy to image
    auto commandBuffers = OneTimeCommandBuffers::Get()->allocCommandBuffers();
    auto commandBuffer = commandBuffers[0];

    vk::ImageSubresourceLayers subresource;
    subresource.setLayerCount(1).setBaseArrayLayer(0)
        .setMipLevel(0).setAspectMask(m_imageAspectFlag);
    vk::BufferImageCopy region;
    region.setBufferImageHeight(0).setBufferRowLength(0)
        .setBufferOffset(0).setImageExtent(m_imageInfo.extent)
        .setImageSubresource(subresource);

    commandBuffer.copyBufferToImage(temporaryBuffer.m_buffer,
        m_image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

    OneTimeCommandBuffers::Get()->executeCommandBufers(commandBuffers, m_vulkanDevice.m_queues.graphicsQueue);
    OneTimeCommandBuffers::Get()->freeCommandBuffers(commandBuffers);

    if (m_mipLevels > 1)
    { // Generate mipmaps
        uint32_t mipWidth = width;
        uint32_t mipHeight = height;
        auto mipmapCommandBuffers = OneTimeCommandBuffers::Get()->allocCommandBuffers();
        commandBuffer = mipmapCommandBuffers[0];
        vk::ImageSubresourceRange texSubresource;
        texSubresource.setAspectMask(m_imageAspectFlag)
            .setBaseArrayLayer(0).setLayerCount(1)
            .setLevelCount(1);
        for (uint32_t i = 1; i < m_mipLevels; ++i)
        {
            texSubresource.setBaseMipLevel(i - 1);
            imageBarrier(texSubresource, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eTransferSrcOptimal,
                m_vulkanDevice.m_families.graphicsIndex, m_vulkanDevice.m_families.graphicsIndex, commandBuffer);

            vk::ImageSubresourceLayers srcLayers, dstLayers;
            srcLayers.setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseArrayLayer(0).setLayerCount(1).setMipLevel(i - 1);
            dstLayers.setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseArrayLayer(0).setLayerCount(1).setMipLevel(i);

            uint32_t newMipWidth = mipWidth > 1 ? mipWidth / 2 : 1;
            uint32_t newMipHeight = mipHeight > 1 ? mipHeight / 2 : 1;
            vk::ImageBlit blit;
            blit.setSrcSubresource(srcLayers).setDstSubresource(dstLayers)
                .setSrcOffsets({ vk::Offset3D(0,0,0), vk::Offset3D(mipWidth,mipHeight,1) })
                .setDstOffsets({ vk::Offset3D(0,0,0), vk::Offset3D(newMipWidth,newMipHeight,1) });

            commandBuffer.blitImage(m_image, vk::ImageLayout::eTransferSrcOptimal,
                m_image, vk::ImageLayout::eTransferDstOptimal, 1, &blit,
                vk::Filter::eLinear);

            imageBarrier(texSubresource, vk::ImageLayout::eTransferSrcOptimal, layout,
                m_vulkanDevice.m_families.graphicsIndex, m_vulkanDevice.m_families.graphicsIndex, commandBuffer);

            if (newMipWidth > 1)
                mipWidth = newMipWidth;
            if (newMipHeight > 1)
                mipHeight = newMipHeight;

        }

        texSubresource.setBaseMipLevel(mipLevels - 1);
        imageBarrier(texSubresource, vk::ImageLayout::eTransferDstOptimal, layout,
            m_vulkanDevice.m_families.graphicsIndex, m_vulkanDevice.m_families.graphicsIndex, commandBuffer);

        OneTimeCommandBuffers::Get()->executeCommandBufers(mipmapCommandBuffers, m_vulkanDevice.m_queues.graphicsQueue);
        OneTimeCommandBuffers::Get()->freeCommandBuffers(mipmapCommandBuffers);
    }
    else
    {
        imageBarrier(range, vk::ImageLayout::eTransferDstOptimal, layout,
            m_vulkanDevice.m_families.graphicsIndex, m_vulkanDevice.m_families.graphicsIndex);
    }

    vmaFreeMemory(g_allocator, temporaryBuffer.m_memory);
    m_vulkanDevice.m_logicalDevice.destroyBuffer(temporaryBuffer.m_buffer);

    createImageView(vk::ImageViewType::e2D);
}

auto Image::createImageView(vk::ImageViewType type) -> void
{
    vk::ImageViewCreateInfo viewInfo;
    viewInfo.setComponents(vk::ComponentMapping())
        .setFormat(m_imageInfo.format).setImage(m_image)
        .setViewType(type)
        .setSubresourceRange(m_subresourceRange);
    m_imageView = m_vulkanDevice.m_logicalDevice.createImageView(viewInfo);
    EVALUATE(m_imageView, nullptr, == , "Couldn't create a image view for texture");
}

auto Image::imageBarrier(vk::ImageSubresourceRange range, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
    uint32_t srcQueueIndex, uint32_t dstQueueIndex, vk::CommandBuffer buff) -> void
{
    vk::CommandBuffer commandBuffer = buff;
    std::vector<vk::CommandBuffer> commandBuffers;
    if (buff == vk::CommandBuffer(nullptr))
    {
        commandBuffers = OneTimeCommandBuffers::Get()->allocCommandBuffers();
        commandBuffer = commandBuffers[0];
    }

    vk::PipelineStageFlags srcStageFlags;
    vk::PipelineStageFlags  dstStageFlags;


    vk::ImageMemoryBarrier barrier;
    barrier.setImage(m_image)
        .setSrcQueueFamilyIndex(srcQueueIndex).setDstQueueFamilyIndex(dstQueueIndex)
        .setOldLayout(oldLayout).setNewLayout(newLayout)
        .setSubresourceRange(range);

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead);
        barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

        srcStageFlags = vk::PipelineStageFlagBits::eTransfer;
        dstStageFlags = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
    {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead);
        barrier.setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

        srcStageFlags = vk::PipelineStageFlagBits::eTransfer;
        dstStageFlags = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    }
    else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferSrcOptimal)
    {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead);
        barrier.setDstAccessMask(vk::AccessFlagBits::eTransferRead);

        srcStageFlags = vk::PipelineStageFlagBits::eTransfer;
        dstStageFlags = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eColorAttachmentOptimal)
    {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead);
        barrier.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentWrite);

        srcStageFlags = vk::PipelineStageFlagBits::eTransfer;
        dstStageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
        barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

        srcStageFlags = vk::PipelineStageFlagBits::eTransfer;
        dstStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eTransferSrcOptimal)
    {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
        barrier.setDstAccessMask(vk::AccessFlagBits::eTransferRead);

        srcStageFlags = vk::PipelineStageFlagBits::eTransfer;
        dstStageFlags = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eTransferSrcOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
        barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

        srcStageFlags = vk::PipelineStageFlagBits::eTransfer;
        dstStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else
        THROW_ERROR("Couldn't use an image barrier from %d layout to %d", (int)oldLayout, (int)newLayout);
 
    commandBuffer.pipelineBarrier(srcStageFlags, dstStageFlags,
        {}, 0, nullptr, 0, nullptr,
        1, &barrier);

    if (buff == vk::CommandBuffer(nullptr))
    {
        OneTimeCommandBuffers::Get()->executeCommandBufers(commandBuffers, m_vulkanDevice.m_queues.graphicsQueue);
        OneTimeCommandBuffers::Get()->freeCommandBuffers(commandBuffers);
    }
}
