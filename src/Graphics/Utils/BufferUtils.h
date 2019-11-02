#pragma once

#include <Oblivion.h>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace BufferUtils
{
    struct Buffer
    {
        vk::Buffer      m_buffer;
        VmaAllocation   m_memory;
        std::size_t     m_size = 0;
    };

    Buffer createBuffer(vk::BufferUsageFlags usage,
        vk::MemoryPropertyFlags requiredMemory, vk::MemoryPropertyFlags preferredMemory,
        uint32_t * families, uint32_t familiesCount, std::size_t size,
        vk::Device device = nullptr, vk::Queue queue = nullptr, uint32_t queueFamilyIndex = ~0,
        void* pData = nullptr);

}