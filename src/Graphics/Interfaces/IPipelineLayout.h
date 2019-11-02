#pragma once


#include "IGraphicsObject.h"
#include "IFrameDependent.h"


class IPipelineLayout :
    public IVulkanDeviceObject
{
public:
    virtual void                                            bindDescriptorSets(vk::CommandBuffer& commandBuffer) const = 0;
    virtual vk::PipelineLayout                              getPipelineLayout() const = 0;
    virtual std::vector<vk::PipelineShaderStageCreateInfo>  getShadersCreateInfo() const = 0;
};