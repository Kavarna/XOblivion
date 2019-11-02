#pragma once


#include <Oblivion.h>

#include "../../Interfaces/IPipelineLayout.h"


#include "../../Utils/BufferUtils.h"
#include "../../Utils/Image.h"

#include "../../Utils/Shader.h"

class TextureLayout :
    public IPipelineLayout
{
    struct UniformBufferObject
    {
        glm::mat4 world;
        glm::mat4 view;
        glm::mat4 projection;
    };
public:
    TextureLayout();
    ~TextureLayout();


    virtual     void                        update();


    virtual     void                        bindDescriptorSets(vk::CommandBuffer& commandBuffer) const;

    virtual     std::vector<vk::PipelineShaderStageCreateInfo> 
                                            getShadersCreateInfo() const override;

    virtual     vk::PipelineLayout          getPipelineLayout() const { return m_layout; };

public:
                auto                        setImage(Image* image, vk::Sampler sampler) -> void;
                auto                        getVertexShader() const -> const Shader& { return m_vertexShader; };
                auto                        getFragmentShader() const -> const Shader& { return m_fragmentShader; };
                auto                        setWorld(const glm::mat4& world) -> void { m_uniformBufferObject.world = world; };
                auto                        setView(const glm::mat4& view) -> void { m_uniformBufferObject.view = view; };
                auto                        setProjection(const glm::mat4& projection) -> void { m_uniformBufferObject.projection = projection; };
private:
                auto                        createDescriptorPools() -> void;
                auto                        allocateDescriptorSets() -> void;
                auto                        updateDescriptorSets() -> void;


private:
    vk::PipelineLayout                      m_layout;
    vk::DescriptorSetLayout                 m_descriptorLayout;
    vk::DescriptorPool                      m_descriptorPool;

    vk::DescriptorSet                       m_descriptorSet;

    Shader                                  m_vertexShader;
    Shader                                  m_fragmentShader;

    uint32_t                                m_hasTexture = 0;

    BufferUtils::Buffer                     m_vertexShaderUniformBuffer;

    UniformBufferObject                     m_uniformBufferObject;

};