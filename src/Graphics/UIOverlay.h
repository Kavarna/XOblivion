#pragma once


#include "../Graphics/Pipeline/Layout/UIOverlayLayout.h"
#include "Interfaces/IGraphicsObject.h"
#include "imgui/imgui.h"
#include "Utils/Shader.h"
#include "Utils/Image.h"
#include "Utils/BufferUtils.h"

class UIOverlay : public IVulkanDeviceObject
{
public:
    UIOverlay(vk::RenderPass renderpass);
    ~UIOverlay();

public:
    auto                                update(float frametime) -> bool;
    auto                                render(vk::CommandBuffer commandBuffer, float width, float height) -> void;

public:
    auto                                setUICallback(std::function<void(float)> callback) -> void;

    auto                                resize(float width, float height) -> void;

public:
    auto                                begin(const std::string& name) -> void;
    auto                                end() -> void;
    
    auto                                text(const std::string& msg) -> void;

private:
    auto                                prepareFont() -> void;
    auto                                preparePipeline(vk::RenderPass) -> void;

private:
    auto                                update() -> bool;

public:
    std::unique_ptr<Image>              m_fontImage;

    BufferUtils::Buffer                 m_vertexBuffer;
    BufferUtils::Buffer                 m_indexBuffer;

    std::function<void(float)>          m_uicallback;

    // Pipeline
    std::unique_ptr<UIOverlayLayout>    m_pipelineLayout;
    vk::Pipeline                        m_pipeline;

};

