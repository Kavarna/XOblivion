#include "UIOverlay.h"


#include "Utils/Samplers.h"

#include "../Core/Input.h"
#include "../Core/Window.h"
#include "VulkanRenderer.h"

UIOverlay::UIOverlay(vk::RenderPass renderpass)
{
    // Init ImGui
    ImGui::CreateContext();
    ImGui::GetIO().FontGlobalScale = 1.0f;
    // Color scheme
    ImGuiStyle& style = ImGui::GetStyle();
    style.AntiAliasedFill = true;
    style.AntiAliasedLines = true;

    prepareFont();
    preparePipeline(renderpass);
}

UIOverlay::~UIOverlay()
{
    ImGui::DestroyContext();

    if (m_vertexBuffer.m_buffer)
    {
        m_vulkanDevice.m_logicalDevice.destroyBuffer(m_vertexBuffer.m_buffer);
        vmaFreeMemory(g_allocator, m_vertexBuffer.m_memory);
    }

    if (m_indexBuffer.m_buffer)
    {
        m_vulkanDevice.m_logicalDevice.destroyBuffer(m_indexBuffer.m_buffer);
        vmaFreeMemory(g_allocator, m_indexBuffer.m_memory);
    }

    if (m_pipelineLayout)
    {
        m_vulkanDevice.m_logicalDevice.destroyPipeline(m_pipeline);
        m_pipelineLayout = nullptr;
    }
    m_pipelineLayout.reset();
    m_fontImage.reset();
}

auto UIOverlay::update(float frametime) -> bool
{
    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = ImVec2(WindowObject::Get()->getWindowWidth(), WindowObject::Get()->getWindowHeight());
    io.DeltaTime = frametime;

    io.MousePos = ImVec2((float)Input::Get()->getMouseX(), (float)Input::Get()->getMouseY());
    io.MouseDown[0] = Input::Get()->getKeyState("LEFT_CLICK") == Input::EKeyState::ePress;
    io.MouseDown[1] = Input::Get()->getKeyState("RIGHT_CLICK") == Input::EKeyState::ePress;

    ImGui::NewFrame();
    // ImGui::Begin("Debug");
    // ImGui::Text("Hello from Dear ImGui");
    // ImGui::End();

    if (m_uicallback)
        m_uicallback(frametime);

    ImGui::Render();

    return update();
}

auto UIOverlay::render(vk::CommandBuffer commandBuffer, float width, float height) -> void
{
    ImDrawData* imDrawData = ImGui::GetDrawData();
    int32_t vertexOffset = 0;
    int32_t indexOffset = 0;

    if ((!imDrawData) || (imDrawData->CmdListsCount == 0)) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();


    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
    m_pipelineLayout->bindDescriptorSets(commandBuffer);
    m_pipelineLayout->setPushConstants(commandBuffer, { 2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y }, { -1.f,-1.0f });

    vk::Viewport viewport(0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f);
    commandBuffer.setViewport(0, 1, &viewport);
    vk::Rect2D scissorRect(vk::Offset2D{}, vk::Extent2D{ (uint32_t)width, (uint32_t)height });
    commandBuffer.setScissor(0, 1, &scissorRect);

    VkDeviceSize offsets[1] = { 0 };
    commandBuffer.bindVertexBuffers(0, 1, &m_vertexBuffer.m_buffer, offsets);
    commandBuffer.bindIndexBuffer(m_indexBuffer.m_buffer, 0, vk::IndexType::eUint16);

    for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
    {
        const ImDrawList* cmd_list = imDrawData->CmdLists[i];
        for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
            commandBuffer.drawIndexed(pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
            
            indexOffset += pcmd->ElemCount;
        }
        vertexOffset += cmd_list->VtxBuffer.Size;
    }
}

auto UIOverlay::setUICallback(std::function<void(float)> callback) -> void
{
    m_uicallback = callback;
}

auto UIOverlay::resize(float width, float height) -> void
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(width, height);
}

auto UIOverlay::prepareFont() -> void
{
    unsigned char* fontData;
    int texWidth, texHeight, imageSize;
    auto& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("Resources/Roboto-Medium.ttf", 16.f);
    io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
    imageSize = texWidth * texHeight * 4 * sizeof(char);

    m_fontImage = std::make_unique<Image>(fontData,
        imageSize, texWidth, texHeight,
        vk::Format::eR8G8B8A8Unorm,
        vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryPropertyFlags(),
        vk::ImageLayout::eShaderReadOnlyOptimal);
}

auto UIOverlay::preparePipeline(vk::RenderPass renderpass) -> void
{
    m_pipelineLayout = std::make_unique<UIOverlayLayout>();
    m_pipelineLayout->setImage(m_fontImage.get(), Samplers::Get()->getLinearAnisotropicSampler());

    vk::PipelineColorBlendAttachmentState blendAttachmentState{};
    blendAttachmentState.setBlendEnable(VK_TRUE)
        .setAlphaBlendOp(vk::BlendOp::eAdd)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
        .setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
            .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
        .setColorWriteMask( vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

    vk::PipelineColorBlendStateCreateInfo blendState;
    blendState.setAttachmentCount(1).setPAttachments(&blendAttachmentState);

    // depth state
    vk::PipelineDepthStencilStateCreateInfo depthState;
    depthState.setDepthTestEnable(VK_FALSE).setStencilTestEnable(VK_FALSE);

    // dynamic state
    vk::PipelineDynamicStateCreateInfo dynamicState;
    vk::DynamicState dynamicStates[] =
    {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };
    dynamicState.setDynamicStateCount(ARRAYSIZE(dynamicStates)).setPDynamicStates(dynamicStates);

    // IA state
    vk::PipelineInputAssemblyStateCreateInfo inputState;
    inputState.setTopology(vk::PrimitiveTopology::eTriangleList).setPrimitiveRestartEnable(VK_FALSE);

    // MSAA state
    vk::PipelineMultisampleStateCreateInfo msaaState;
    msaaState.setAlphaToCoverageEnable(VK_FALSE).setAlphaToOneEnable(VK_FALSE)
        .setSampleShadingEnable(VK_FALSE).setMinSampleShading(0.0f)
        .setPSampleMask(nullptr).setRasterizationSamples(m_vulkanDevice.m_bestSampling);

    // Rasterizer state
    vk::PipelineRasterizationStateCreateInfo rasterizerState;
    rasterizerState.setCullMode(vk::CullModeFlagBits::eNone).setPolygonMode(vk::PolygonMode::eFill).
        setFrontFace(vk::FrontFace::eCounterClockwise).setDepthBiasEnable(VK_FALSE).setDepthBiasClamp(0.0f).
        setDepthBiasConstantFactor(0.0f).setDepthBiasSlopeFactor(0.0f).setDepthClampEnable(VK_FALSE).
        setLineWidth(1.0f).setRasterizerDiscardEnable(VK_FALSE);

    // Viewport state
    vk::PipelineViewportStateCreateInfo viewportState;
    viewportState.setScissorCount(1).setViewportCount(1);

    // Pipeline Info
    vk::VertexInputBindingDescription binding;
    binding.setBinding(0).setInputRate(vk::VertexInputRate::eVertex).setStride(sizeof(ImDrawVert));
    vk::VertexInputAttributeDescription attributes[]
    {
                                    // Location      Binding                 Format                         offset
        vk::VertexInputAttributeDescription(0,          0,        vk::Format::eR32G32Sfloat,            offsetof(ImDrawVert, pos)),
        vk::VertexInputAttributeDescription(1,          0,        vk::Format::eR32G32Sfloat,            offsetof(ImDrawVert, uv)),
        vk::VertexInputAttributeDescription(2,          0,        vk::Format::eR8G8B8A8Unorm,           offsetof(ImDrawVert, col))
    };

    vk::PipelineVertexInputStateCreateInfo vertexInputState;
    vertexInputState.setVertexBindingDescriptionCount(1).setPVertexBindingDescriptions(&binding)
        .setVertexAttributeDescriptionCount(ARRAYSIZE(attributes)).setPVertexAttributeDescriptions(attributes);


    auto stages = m_pipelineLayout->getShadersCreateInfo();
    vk::GraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.setBasePipelineHandle(nullptr).setBasePipelineIndex(0)
        .setPColorBlendState(&blendState)
        .setPDepthStencilState(&depthState)
        .setPDynamicState(&dynamicState)
        .setPInputAssemblyState(&inputState)
        .setPMultisampleState(&msaaState)
        .setPRasterizationState(&rasterizerState)
        .setPTessellationState(nullptr)
        .setPViewportState(&viewportState)
        .setPVertexInputState(&vertexInputState)
        .setLayout(m_pipelineLayout->getPipelineLayout())
        .setStageCount((uint32_t)stages.size()).setPStages(stages.data())
        .setRenderPass(renderpass).setSubpass(0);
    EVALUATE(m_pipeline = m_vulkanDevice.m_logicalDevice.createGraphicsPipeline(nullptr, pipelineInfo),
        nullptr, == , "Unable to create a graphics pipeline for UIOverlay");
}

auto UIOverlay::update() -> bool
{
    ImDrawData* imDrawData = ImGui::GetDrawData();
    bool updated = false;

    if (!imDrawData) { return false; }

    vk::DeviceSize vertexBuffSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
    vk::DeviceSize indexBuffSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
    if (vertexBuffSize == 0 || indexBuffSize == 0) { return false; }

    VulkanRenderer::Get()->wait();

    if (vertexBuffSize != m_vertexBuffer.m_size)
    {
        
        if (m_vertexBuffer.m_buffer)
        {
            m_vulkanDevice.m_logicalDevice.destroyBuffer(m_vertexBuffer.m_buffer);
            vmaFreeMemory(g_allocator, m_vertexBuffer.m_memory);
        }

        m_vertexBuffer = BufferUtils::createBuffer(vk::BufferUsageFlagBits::eVertexBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible, {},
            &m_vulkanDevice.m_families.graphicsIndex, 1, vertexBuffSize);
        updated = true;
    }

    if (indexBuffSize != m_indexBuffer.m_size)
    {
        if (m_indexBuffer.m_buffer)
        {
            m_vulkanDevice.m_logicalDevice.destroyBuffer(m_indexBuffer.m_buffer);
            vmaFreeMemory(g_allocator, m_indexBuffer.m_memory);
        }
        m_indexBuffer = BufferUtils::createBuffer(vk::BufferUsageFlagBits::eIndexBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible, {},
            &m_vulkanDevice.m_families.graphicsIndex, 1, indexBuffSize);
        updated = true;
    }

    // Upload data
    ImDrawVert* vtxDest;
    vmaMapMemory(g_allocator, m_vertexBuffer.m_memory, (void**)&vtxDest);
    ImDrawIdx* idxDest;
    vmaMapMemory(g_allocator, m_indexBuffer.m_memory, (void**)&idxDest);

    for (int i = 0; i < imDrawData->CmdListsCount; ++i)
    {
        const ImDrawList* cmdList = imDrawData->CmdLists[i];
        memcpy(vtxDest, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(idxDest, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
        vtxDest += cmdList->VtxBuffer.Size;
        idxDest += cmdList->IdxBuffer.Size;
    }
    vmaUnmapMemory(g_allocator, m_vertexBuffer.m_memory);
    vmaUnmapMemory(g_allocator, m_indexBuffer.m_memory);

    vmaFlushAllocation(g_allocator, m_vertexBuffer.m_memory, 0, m_vertexBuffer.m_size);
    vmaFlushAllocation(g_allocator, m_indexBuffer.m_memory, 0, m_indexBuffer.m_size);

    return updated;
}

auto UIOverlay::begin(const std::string& name) -> void
{
    ImGui::Begin(name.c_str());
}

auto UIOverlay::end() -> void
{
    ImGui::End();
}

auto UIOverlay::text(const std::string& msg) -> void
{
    ImGui::Text(msg.c_str());
}
