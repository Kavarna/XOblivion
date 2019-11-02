#pragma once


#include "../Graphics/Interfaces/IGraphicsScene.h"
#include "../Graphics/Interfaces/IFrameDependent.h"
#include "../Graphics/Pipeline/Layout/TextureLayout.h"
#include "../Graphics/Vertex/PositionColorVertex.h"
#include "../Graphics/Pipeline/SimplePipeline.h"
#include "../Graphics/Utils/Image.h"
#include "../Graphics/Model.h"
#include "../Graphics/UIOverlay.h"

#include "../Gameplay/FirstPersonCamera.h"

class SimpleScene :
    public IGraphicsScene, public IFrameDependent
{
    using Pipeline = SimplePipeline<TextureLayout, PositionColorVertex>;
public:
    SimpleScene();
    ~SimpleScene();

    // Inherited via IGraphicsScene
    virtual std::vector<vk::CommandBuffer>  getCommandBuffers(uint32_t currentFrame) override;

    auto                                    update(float frametime) -> void;

    // Inherited via IFrameDependent
    virtual void create(uint32_t totalFrames, uint32_t width, uint32_t height) override;
    virtual void render(uint32_t frameIndex) override;
    virtual void frameCleanup() override;

private:
    auto                            createRenderPass() -> void;
    auto                            createCommandPool() -> void;
    auto                            createPipeline() -> void;
    auto                            loadModels() -> void;

    auto                            createFramebuffers(uint32_t totalFrames, uint32_t width, uint32_t height) -> void;
    auto                            cleanupFramebuffers() -> void;

    auto                            allocateCommandBuffers(uint32_t totalFrames, uint32_t width, uint32_t height) -> void;
    auto                            recordCommandBuffers() -> void;
    auto                            cleanupCommandBuffers() -> void;

    auto                            renderOverlay(vk::CommandBuffer) -> void;
    auto                            renderUI(float frametime) -> void;

public:
    // TODO: Make wrapper over RenderPass and framebuffers
    vk::RenderPass                  m_renderPass;
    std::unique_ptr<Image>          m_depthImage;
    std::unique_ptr<Image>          m_colorImage;
    std::vector<vk::Framebuffer>    m_framebuffers;


    // Command pool and buffers
    vk::CommandPool                 m_graphicsCommandPool;
    std::vector<vk::CommandBuffer>  m_commandBuffers;

    // Models
    std::unique_ptr<Model>          m_model;
    std::unique_ptr<Image>          m_testImage;

    std::unique_ptr<UIOverlay>      m_overlay;


    // Pipeline info
    std::unique_ptr<TextureLayout>  m_textureLayout;
    std::unique_ptr<Pipeline>       m_pipeline;


    std::unique_ptr<FirstPersonCamera>
                                    m_camera;
};

