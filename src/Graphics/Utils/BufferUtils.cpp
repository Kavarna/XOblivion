#include "BufferUtils.h"

#include "VulkanAllocators.h"
#include "OneTimeCommandBuffers.h"

namespace BufferUtils
{

    Buffer createBuffer(vk::BufferUsageFlags usage,
        vk::MemoryPropertyFlags requiredMemory, vk::MemoryPropertyFlags preferredMemory,
        uint32_t * families, uint32_t familiesCount, std::size_t size,
        vk::Device device, vk::Queue queue, uint32_t quueueFamilyIndex,
        void* pData )
    {
        if (pData)
        {
            usage |= vk::BufferUsageFlagBits::eTransferDst;
        }
        vk::BufferCreateInfo bufferInfo = {};
        bufferInfo.setPQueueFamilyIndices(families).setQueueFamilyIndexCount(familiesCount)
            .setSharingMode(familiesCount == 1 ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent)
            .setUsage(usage).setSize(size);

        vk::Buffer buffer;
        VmaAllocationCreateInfo allocationInfo = {};
        allocationInfo.requiredFlags = (VkMemoryPropertyFlags)requiredMemory;
        allocationInfo.preferredFlags = (VkMemoryPropertyFlags)preferredMemory;
        VmaAllocation allocation;
        VkResult res = vmaCreateBuffer(g_allocator, (VkBufferCreateInfo*)&bufferInfo, &allocationInfo,
            (VkBuffer*)&buffer, &allocation, nullptr);
        EVALUATE(res, VkResult::VK_SUCCESS, != , "Couldn't create a valid buffer");
        Buffer result = { buffer,allocation,size };

        if (pData)
        { // WE HAVE DATA TO MAP INTO THE BUFFER
            if (requiredMemory & vk::MemoryPropertyFlagBits::eHostVisible)
            { //  We can copy straight into this buffer
                void* data;
                vmaMapMemory(g_allocator, allocation, &data);
                memcpy(data, pData, size);
                vmaUnmapMemory(g_allocator, allocation);
            }
            else
            {
                bufferInfo.setPQueueFamilyIndices(&quueueFamilyIndex).setQueueFamilyIndexCount(1)
                    .setSharingMode(vk::SharingMode::eExclusive).setUsage(vk::BufferUsageFlagBits::eTransferSrc);
                vk::Buffer tempBuff;
                VmaAllocationCreateInfo allocationInfo = {};
                allocationInfo.requiredFlags = (VkMemoryPropertyFlags)vk::MemoryPropertyFlagBits::eHostVisible;
                allocationInfo.preferredFlags = (VkMemoryPropertyFlags)vk::MemoryPropertyFlagBits::eHostCoherent;
                VmaAllocation temporaryAllocation;
                VkResult res = vmaCreateBuffer(g_allocator,
                    (VkBufferCreateInfo*)&bufferInfo, &allocationInfo,
                    (VkBuffer*)&tempBuff, &temporaryAllocation, nullptr);
                EVALUATE(res, VkResult::VK_SUCCESS, != , "Couldn't create a valid transfer buffer");

                void * data;
                vmaMapMemory(g_allocator, temporaryAllocation, &data);
                memcpy(data, pData, size);
                vmaUnmapMemory(g_allocator, temporaryAllocation);


                auto commandBuffers = OneTimeCommandBuffers::Get()->allocCommandBuffers<>();
                auto commandBuffer = commandBuffers[0];

                vk::BufferCopy copyInfo;
                copyInfo.setSrcOffset(0).setDstOffset(0).setSize(size);
                commandBuffer.copyBuffer(tempBuff, buffer, 1, &copyInfo);


                OneTimeCommandBuffers::Get()->executeCommandBufers(commandBuffers, queue);
                OneTimeCommandBuffers::Get()->freeCommandBuffers<>(commandBuffers);

                device.destroyBuffer(tempBuff);
                vmaFreeMemory(g_allocator, temporaryAllocation);
            }
        }

        return result;

    }


}