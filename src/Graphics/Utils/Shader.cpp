#include "Shader.h"

#include <cstdio>
#include <boost/algorithm/string.hpp>


Shader::Shader(const std::string & path)
{
    FILE * f = fopen(path.c_str(), "rb");

    if (!f)
        THROW_INITIALIZATION_EXCEPTION("Couldn't find file %s", path.c_str());

    fseek(f, 0, SEEK_END);
    auto size = ftell(f);
    fseek(f, 0, SEEK_SET);
    m_rawData = new unsigned char[size];
    
    fread(m_rawData, sizeof(unsigned char), size, f);
    fclose(f);

    vk::ShaderModuleCreateInfo shaderInfo;
    shaderInfo.setCodeSize(size);
    shaderInfo.setPCode(reinterpret_cast<const uint32_t*>(m_rawData));
    m_shader = m_vulkanDevice.m_logicalDevice.createShaderModule(shaderInfo);
    if (!m_shader)
        THROW_INITIALIZATION_EXCEPTION("Couldn't create Shader for file %s", path.c_str());
    if (boost::contains(path, "vert"))
        m_shaderType = vk::ShaderStageFlagBits::eVertex;
    else if (boost::contains(path, "frag"))
        m_shaderType = vk::ShaderStageFlagBits::eFragment;


    m_pipelineCreateInfo.setModule(m_shader);
    m_pipelineCreateInfo.setPName("main");
    m_pipelineCreateInfo.setStage(m_shaderType);
    m_pipelineCreateInfo.setPSpecializationInfo(nullptr);
}

Shader::~Shader()
{
    destroy();
}

void Shader::destroy()
{
    if (m_shader)
    {
        m_vulkanDevice.m_logicalDevice.destroyShaderModule(m_shader);
        m_shader = nullptr;
    }
}