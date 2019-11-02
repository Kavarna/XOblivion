#pragma once

#include "Oblivion.h"
#include <vulkan/vulkan.hpp>

#include "Utils/Image.h"
#include "Vertex/PositionVertex.h"
#include "Vertex/PositionColorVertex.h"
#include "Utils/BufferUtils.h"
#include "Utils/VulkanObjects.h"
#include "Interfaces/IFrameDependent.h"
#include "Interfaces/IGraphicsScene.h"

#include "Pipeline/Layout/TextureLayout.h"


class VulkanRenderer : public ISingletone<VulkanRenderer>
{
    static constexpr const uint8_t _maxInFlightFrames = 2;
    template <typename T>
    using InFlightArray = std::array<T, _maxInFlightFrames>;
public:
    VulkanRenderer();
    ~VulkanRenderer();

public:
    auto									create(uint32_t, uint32_t) -> void;
    auto									onSize(uint32_t, uint32_t) -> void;
    auto                                    acquire() -> void;
    auto									render(IGraphicsScene* scene) -> void;
    auto                                    present() -> void;
    auto                                    wait() -> void;
    auto                                    addFrameDependentObject(IFrameDependent* object) -> void;


public:
    auto									addInstanceLayer(const char*) -> bool;
    auto									addInstanceExtension(const char*) -> bool;

    auto									getPresentInstanceLayers() const->std::vector<vk::LayerProperties>;
    auto									getPresentInstanceExtensions() const->std::vector<vk::ExtensionProperties>;

public:
    auto									getVulkanInstance() const -> const vk::Instance&;
    auto                                    getVulkanLogicalDevice() const -> const vk::Device&;
    auto                                    getVulkanDeviceInfo() const -> const DeviceInfo&;
    auto                                    getVulkanSwapchainInfo() const -> const SwapchainInfo&;
    auto                                    getVulkanSwapchainCreateInfo() const -> const SwapchainCreateInfo&;

private:
    auto									createInstance() -> void;
    auto									createSurface() -> void;
    auto									createDevice() -> void;
    auto                                    createAllocators() -> void;
    auto									createSwapchain(uint32_t, uint32_t) -> void;
    auto									createSyncObjects() -> void;

private:
    auto                                    updateFrameDependentObjects(uint32_t currentImage) -> void;

private:
    auto									querySwapchainCreateInfo(uint32_t width, uint32_t height) -> void;

private:
    auto                                    destroyUtilities() -> void;

private:
    auto									selectExtent(uint32_t width, uint32_t height)->vk::Extent2D;
    auto									selectFormat()->vk::SurfaceFormatKHR;
    auto									selectPresentMode()->vk::PresentModeKHR;
    auto									clearSwapchainImageViews() -> void;

public: // TODO: Make update things depending on these values
    bool                                    m_hasVsync = true;

private:
    vk::Instance							m_vulkanInstance;

    DeviceInfo                              m_vulkanDevice;

    SwapchainCapabilities                   m_swapchainCapabilities;

    SwapchainCreateInfo                     m_swapchainCreateInfo;
    vk::SurfaceKHR							m_renderingSurface;
    SwapchainInfo                           m_swapchainInfo;
    vk::SwapchainKHR						m_swapchain;


    InFlightArray<vk::Semaphore>            m_imageAvailableSemaphore;
    InFlightArray<vk::Semaphore>            m_renderingFinishedSemaphore;
    InFlightArray<vk::Fence>                m_inFlightFence;

    
private:
    std::vector<IFrameDependent*>           m_frameDependentObjects;
    uint32_t                                m_inFlightFrame = 0;
    uint32_t                                m_currentFrame = 0;

private:
    std::vector<const char*>				m_enabledLayers;
    std::vector<const char*>				m_enabledExtensions;

    std::vector<vk::LayerProperties>		m_presentLayers;
    std::vector<vk::ExtensionProperties>	m_presentExtensions;
};