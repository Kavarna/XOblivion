#pragma once

#include <Oblivion.h>
#include <vulkan/vulkan.hpp>
#include "../Interfaces/IGraphicsObject.h"
#include "VulkanObjects.h"

class OneTimeCommandBuffers : public IVulkanDeviceObject, public ISingletone<OneTimeCommandBuffers>
{

public:
    OneTimeCommandBuffers() : 
        IVulkanDeviceObject()
    {
        vk::CommandPoolCreateInfo commandPoolInfo = {};
        // Graphics command pool
        commandPoolInfo.setQueueFamilyIndex(m_vulkanDevice.m_families.graphicsIndex);
        m_graphicsCommandPool = m_vulkanDevice.m_logicalDevice.createCommandPool(commandPoolInfo);
        EVALUATE(m_graphicsCommandPool, nullptr, == , "Couldn't create a one-time graphics command pool");

        // Present command pool
        commandPoolInfo.setQueueFamilyIndex(m_vulkanDevice.m_families.presentIndex);
        m_presentCommandPool = m_vulkanDevice.m_logicalDevice.createCommandPool(commandPoolInfo);
        EVALUATE(m_presentCommandPool, nullptr, == , "Couldn't create a one-time present command pool");

    }
    ~OneTimeCommandBuffers()
    {
        m_vulkanDevice.m_logicalDevice.destroyCommandPool(m_graphicsCommandPool);
        m_vulkanDevice.m_logicalDevice.destroyCommandPool(m_presentCommandPool);
    }


    /// <summary>
    ///     Allocates from the pool numCommandBuffers and calls begin on every of them
    /// </summary>
    /// <param name = "numCommandBuffers">Number of command buffers to allocate (and begin)</param>
    /// <param name = "level">The kind of command buffers (primary or secondary)</param>
    /// <param name = "usage">The way to use the buffers</param>
    template <QueueFamilyType type = QueueFamilyType::eGraphics>
    std::vector<vk::CommandBuffer>  allocCommandBuffers(
        uint32_t numCommandBuffers = 1, vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary,
        vk::CommandBufferUsageFlagBits usage = vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
    {
        vk::CommandBufferAllocateInfo allocationInfo = {};
        allocationInfo.setCommandBufferCount(numCommandBuffers).setLevel(level);
        if constexpr (type == QueueFamilyType::eGraphics)
        {
            allocationInfo.setCommandPool(m_graphicsCommandPool);
        }
        else if constexpr (type == QueueFamilyType::ePresent)
        {
            allocationInfo.setCommandPool(m_presentCommandPool);
        }

        auto buffers = m_vulkanDevice.m_logicalDevice.allocateCommandBuffers(allocationInfo);
        EVALUATE(buffers.size(), 0, == , "Couldn't create %d one time command buffers", numCommandBuffers);

        vk::CommandBufferBeginInfo beginInfo = {};
        beginInfo.setFlags(usage);
        for (auto it : buffers)
        {
            it.begin(beginInfo);
        }

        return buffers;
    }

    void                            executeCommandBufers(const std::vector<vk::CommandBuffer>& buffers, vk::Queue queue)
    {
        for (const auto it : buffers)
            it.end();

        vk::SubmitInfo submitInfo;
        submitInfo.setCommandBufferCount((uint32_t)buffers.size()).setPCommandBuffers(buffers.data())
            .setSignalSemaphoreCount(0).setWaitSemaphoreCount(0);

        queue.submit(submitInfo, nullptr);
        queue.waitIdle();
    }

    /// <summary>
    /// Frees One Time Command Buffers allocated previously<br/>
    /// !THE DEVICE AND POOL MUST BE THE SAME FROM ALLOCATION
    /// </summary>
    /// <param name = "buffers">The buffers to delete</param>
    template <QueueFamilyType type = QueueFamilyType::eGraphics>
    void                            freeCommandBuffers(std::vector<vk::CommandBuffer>& buffers)
    {
        if constexpr (type == QueueFamilyType::eGraphics)
        {
            m_vulkanDevice.m_logicalDevice.freeCommandBuffers(m_graphicsCommandPool, buffers);
        }
        else if constexpr (type == QueueFamilyType::ePresent)
        {
            m_vulkanDevice.m_logicalDevice.freeCommandBuffers(m_presentCommandPool, buffers);
        }
        buffers.clear();
    }

private:
    vk::CommandPool                 m_graphicsCommandPool;
    vk::CommandPool                 m_presentCommandPool;

};