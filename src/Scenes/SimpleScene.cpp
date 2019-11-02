#include "SimpleScene.h"
#include "../Graphics/VulkanRenderer.h"
#include "../Graphics/Utils/Samplers.h"
#include "../Graphics/Utils/VulkanAllocators.h"

#include "../Core/Input.h"
#include "../Core/Window.h"

SimpleScene::SimpleScene()
{
    createRenderPass();
    createCommandPool();
    createPipeline();
    loadModels();
    //WindowObject::Get()->toggleMouse();

}

SimpleScene::~SimpleScene()
{
    frameCleanup();
    m_overlay.reset();
    m_textureLayout.reset();
    m_pipeline.reset();
    m_model.reset();
    m_testImage.reset();
    if (m_graphicsCommandPool)
    {
        m_vulkanDevice.m_logicalDevice.destroyCommandPool(m_graphicsCommandPool);
        m_graphicsCommandPool = nullptr;
    }
    if (m_renderPass)
    {
        m_vulkanDevice.m_logicalDevice.destroyRenderPass(m_renderPass);
        m_renderPass = nullptr;
    }
}

std::vector<vk::CommandBuffer> SimpleScene::getCommandBuffers(uint32_t currentFrame)
{
    return std::vector<vk::CommandBuffer>{m_commandBuffers[currentFrame]};
}

void SimpleScene::create(uint32_t totalFrames, uint32_t width, uint32_t height)
{
    m_overlay->resize((float)width, (float)height);
    m_camera->setAspectRatio(glm::radians(60.f), (float)4.f / 3.f, 0.1f, 1000.f);
    createFramebuffers(totalFrames, width, height);
    m_pipeline->create(totalFrames, width, height); 
    m_textureLayout->setImage(m_testImage.get(), Samplers::Get()->getLinearAnisotropicSampler());
    allocateCommandBuffers(totalFrames, width, height);
}

void SimpleScene::render(uint32_t frameIndex)
{
    m_pipeline->render(frameIndex);
}

void SimpleScene::frameCleanup()
{
    m_pipeline->frameCleanup();
    cleanupCommandBuffers();
    cleanupFramebuffers();
}

auto SimpleScene::update(float frameTime) -> void
{
    if (WindowObject::Get()->mouseEnabled())
    {

    }
    else
    {
        if (Input::Get()->getKeyState("w") == Input::EKeyState::ePress ||
            Input::Get()->getKeyState("w") == Input::EKeyState::eRepeat)
        {
            m_camera->moveForward(frameTime);
        }
        if (Input::Get()->getKeyState("s") == Input::EKeyState::ePress ||
            Input::Get()->getKeyState("s") == Input::EKeyState::eRepeat)
        {
            m_camera->moveBackward(frameTime);
        }
        if (Input::Get()->getKeyState("d") == Input::EKeyState::ePress ||
            Input::Get()->getKeyState("d") == Input::EKeyState::eRepeat)
        {
            m_camera->moveRight(frameTime);
        }
        if (Input::Get()->getKeyState("a") == Input::EKeyState::ePress ||
            Input::Get()->getKeyState("a") == Input::EKeyState::eRepeat)
        {
            m_camera->moveLeft(frameTime);
        }

        if (Input::Get()->getKeyState("q") == Input::EKeyState::ePress ||
            Input::Get()->getKeyState("q") == Input::EKeyState::eRepeat)
        {
            m_camera->riseUp(frameTime);
        }
        if (Input::Get()->getKeyState("e") == Input::EKeyState::ePress ||
            Input::Get()->getKeyState("e") == Input::EKeyState::eRepeat)
        {
            m_camera->riseUp(-frameTime);
        }
        m_camera->rotateRight(frameTime, (float)Input::Get()->getMouseX());
        m_camera->rotateUp(frameTime, (float)Input::Get()->getMouseY());
    }
    m_camera->construct();
    m_textureLayout->setWorld(glm::mat4(1.0f));
    m_textureLayout->setView(m_camera->getView());
    m_textureLayout->setProjection(m_camera->getProjection());
    m_textureLayout->update();

    if (m_overlay->update(frameTime))
    { // Rerecord command buffers
        Logger::printToConsole("\n\nImGui requested redraw\n\n");
        recordCommandBuffers();
    }
}

auto SimpleScene::createRenderPass() -> void
{
    auto format = VulkanRenderer::Get()->getVulkanSwapchainCreateInfo().m_format.format;
    vk::AttachmentDescription colorDescription = {};
    colorDescription.setFormat(format).setSamples(m_vulkanDevice.m_bestSampling)
        .setInitialLayout(vk::ImageLayout::eUndefined).setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

    vk::AttachmentReference colorReference = {};
    colorReference.setAttachment(0);
    colorReference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::AttachmentDescription depthDescription = {};
    depthDescription.setFormat(vk::Format::eD32Sfloat).setSamples(m_vulkanDevice.m_bestSampling)
        .setInitialLayout(vk::ImageLayout::eUndefined).setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
        .setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

    vk::AttachmentReference depthReference = {};
    depthReference.setAttachment(1);
    depthReference.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::AttachmentReference resolveReference = {};
    resolveReference.setAttachment(2);
    resolveReference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::AttachmentDescription resolveDescription = {};
    resolveDescription.setFormat(format).setSamples(vk::SampleCountFlagBits::e1)
        .setInitialLayout(vk::ImageLayout::eUndefined).setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
        .setLoadOp(vk::AttachmentLoadOp::eDontCare).setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

    vk::SubpassDescription subpassDescription;
    subpassDescription.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
        .setColorAttachmentCount(1).setPColorAttachments(&colorReference)
        .setInputAttachmentCount(0).setPInputAttachments(nullptr)
        .setPreserveAttachmentCount(0).setPPreserveAttachments(nullptr)
        .setPDepthStencilAttachment(&depthReference)
        .setPResolveAttachments(&resolveReference);

    std::array<vk::AttachmentDescription, 3> attachments = { colorDescription, depthDescription, resolveDescription };
    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.setAttachmentCount((uint32_t)attachments.size()).setPAttachments(attachments.data())
        .setDependencyCount(0).setPDependencies(nullptr)
        .setSubpassCount(1).setPSubpasses(&subpassDescription);

    m_renderPass = m_vulkanDevice.m_logicalDevice.createRenderPass(renderPassInfo);
    EVALUATE(m_renderPass, nullptr, == , "Couldn't create a render pass");
}

auto SimpleScene::createCommandPool() -> void
{
    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.setQueueFamilyIndex(m_vulkanDevice.m_families.graphicsIndex)
        .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    m_graphicsCommandPool = m_vulkanDevice.m_logicalDevice.createCommandPool(poolInfo);
    EVALUATE(m_graphicsCommandPool, nullptr, == , "Couldn't create a graphics command pool");
}

auto SimpleScene::createPipeline() -> void
{
    m_textureLayout = std::make_unique<TextureLayout>();
    m_pipeline = std::make_unique<Pipeline>(m_textureLayout.get(),
        m_renderPass, 0);
}

auto SimpleScene::loadModels() -> void
{
    m_testImage = std::make_unique<Image>("Resources/test.jpg",
        vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryPropertyFlagBits());

    m_model = std::make_unique<Model>("Resources/Cube.obj");
    m_camera = std::make_unique<FirstPersonCamera>(glm::radians(60.f), (float)4.f/3.f, 0.1f, 1000.f);

    m_overlay = std::make_unique<UIOverlay>(m_renderPass);
    m_overlay->setUICallback(std::bind(&SimpleScene::renderUI, this, std::placeholders::_1));
}

auto SimpleScene::createFramebuffers(uint32_t totalFrames, uint32_t width, uint32_t height) -> void
{
    auto swapchainCreateInfo = VulkanRenderer::Get()->getVulkanSwapchainCreateInfo();
    auto swapchainInfo = VulkanRenderer::Get()->getVulkanSwapchainInfo();
    // Create depth image
    m_depthImage = std::make_unique<Image>(width, height,
        vk::Format::eD32Sfloat, vk::ImageUsageFlagBits::eDepthStencilAttachment,
        vk::ImageLayout::eDepthStencilAttachmentOptimal,
        vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryPropertyFlagBits(),
        m_vulkanDevice.m_bestSampling);

    // Create color image
    m_colorImage = std::make_unique<Image>(width, height,
        swapchainCreateInfo.m_format.format,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryPropertyFlagBits(),
        m_vulkanDevice.m_bestSampling);

    // Create framebuffers for images
    m_framebuffers.reserve(totalFrames);
    for (const auto imageView : swapchainInfo.m_imageViews)
    {
        std::array<vk::ImageView, 3> attachments =
        {
            m_colorImage->getImageView(),
            m_depthImage->getImageView(),
            imageView
        };
        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.setAttachmentCount((uint32_t)attachments.size()).setPAttachments(attachments.data())
            .setWidth(width).setHeight(height).setLayers(1)
            .setRenderPass(m_renderPass);

        vk::Framebuffer framebuffer;
        framebuffer = m_vulkanDevice.m_logicalDevice.createFramebuffer(framebufferInfo);
        EVALUATE(framebuffer, nullptr, == , "Couldn't create a frame buffer for a swapchain image");
        m_framebuffers.push_back(framebuffer);
    }
}

auto SimpleScene::cleanupFramebuffers() -> void
{
    m_depthImage.reset();
    m_colorImage.reset();

    for (const auto it : m_framebuffers)
    {
        m_vulkanDevice.m_logicalDevice.destroyFramebuffer(it);
    }
    m_framebuffers.clear();
}

auto SimpleScene::allocateCommandBuffers(uint32_t totalFrames, uint32_t width, uint32_t height) -> void
{
    vk::CommandBufferAllocateInfo allocationInfo = {};
    allocationInfo.setCommandBufferCount(totalFrames);
    allocationInfo.setCommandPool(m_graphicsCommandPool);
    allocationInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    m_commandBuffers = m_vulkanDevice.m_logicalDevice.allocateCommandBuffers(allocationInfo);
    EVALUATE(m_commandBuffers.size(), 0, == , "Couldn't create command buffers");
    recordCommandBuffers();
}

auto SimpleScene::recordCommandBuffers() -> void
{
    VulkanRenderer::Get()->wait();
    auto swapchainCreateInfo = VulkanRenderer::Get()->getVulkanSwapchainCreateInfo();
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0].color.setFloat32({ 0.0f,0.0f,0.0f,1.0f });
    clearValues[1].depthStencil.setDepth(1.0f);
    clearValues[1].depthStencil.setStencil(0);
    vk::ImageSubresourceRange range;
    range.setAspectMask(vk::ImageAspectFlagBits::eColor);
    range.setBaseArrayLayer(0);
    range.setBaseMipLevel(0);
    range.setLayerCount(1);
    range.setLevelCount(1);
    for (size_t i = 0; i < m_commandBuffers.size(); ++i)
    {
        m_commandBuffers[i].begin(beginInfo);

        vk::RenderPassBeginInfo renderPassBeginInfo;
        renderPassBeginInfo.setClearValueCount((uint32_t)clearValues.size()).setPClearValues(clearValues.data())
            .setRenderPass(m_renderPass).setRenderArea({ {0u, 0u}, swapchainCreateInfo.m_extent })
            .setFramebuffer(m_framebuffers[i]);

        m_commandBuffers[i].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
        m_commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline->getPipeline());

        m_textureLayout->bindDescriptorSets(m_commandBuffers[i]);
        m_model->bind(m_commandBuffers[i]);

        m_commandBuffers[i].drawIndexed(m_model->getIndexCount(), 1, 0, 0, 0);

        renderOverlay(m_commandBuffers[i]);

        m_commandBuffers[i].endRenderPass();

        m_commandBuffers[i].end();

    }
}

auto SimpleScene::cleanupCommandBuffers() -> void
{
    if (m_commandBuffers.size())
    {
        m_vulkanDevice.m_logicalDevice.freeCommandBuffers(m_graphicsCommandPool, m_commandBuffers);
        m_commandBuffers.clear();
    }
}

auto SimpleScene::renderOverlay(vk::CommandBuffer cmd) -> void
{
    auto width = WindowObject::Get()->getWindowWidth(), height = WindowObject::Get()->getWindowHeight();
    m_overlay->render(cmd, width, height);
}

auto SimpleScene::renderUI(float frameTime) -> void
{
    m_overlay->begin("SimpleScene");
    m_overlay->text(appendToString("SimpleScene: framtime = ", frameTime));
    m_overlay->end();
}