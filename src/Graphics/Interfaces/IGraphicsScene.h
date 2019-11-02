#pragma once


#include "IGraphicsObject.h"


class IGraphicsScene : 
    public IVulkanDeviceObject
{
public:
    virtual std::vector<vk::CommandBuffer> getCommandBuffers(uint32_t currentFrame) = 0;
};