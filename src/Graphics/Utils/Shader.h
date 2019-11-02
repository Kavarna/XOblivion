#pragma once

#include "vulkan/vulkan.hpp"
#include "Oblivion.h"

#include "../Interfaces/IGraphicsObject.h"

#include <string>


class Shader : public IVulkanDeviceObject
{
public:
    Shader(const std::string& path);
    ~Shader();

public:
    void destroy();

public:
    inline			vk::ShaderModule					getShader() const { return m_shader; };
    inline const	vk::PipelineShaderStageCreateInfo	getShaderStageCreateInfo() const { return m_pipelineCreateInfo; }

private:
    unsigned char*							m_rawData;
    vk::ShaderModule						m_shader;
    vk::PipelineShaderStageCreateInfo		m_pipelineCreateInfo;
    vk::ShaderStageFlagBits					m_shaderType;
};