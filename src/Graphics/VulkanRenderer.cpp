#include "VulkanRenderer.h"
#include "VulkanDebug.h"
#include "Utils/VulkanAllocators.h"
#include "Utils/BufferUtils.h"
#include "Utils/OneTimeCommandBuffers.h"

#include "../Core/Window.h"

#include "Utils/Samplers.h"


std::vector<const char*> deviceEnabledLayers = 
{
};

std::vector<const char*> deviceEnabledExtensions = 
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VulkanRenderer::VulkanRenderer()
{
    m_presentLayers = vk::enumerateInstanceLayerProperties();
    m_presentExtensions = vk::enumerateInstanceExtensionProperties();
}

VulkanRenderer::~VulkanRenderer()
{
    destroyUtilities();

    for (uint32_t i = 0; i < _maxInFlightFrames; ++i)
    {
        m_vulkanDevice.m_logicalDevice.destroySemaphore(m_imageAvailableSemaphore[i]);
        m_vulkanDevice.m_logicalDevice.destroySemaphore(m_renderingFinishedSemaphore[i]);
        m_vulkanDevice.m_logicalDevice.destroyFence(m_inFlightFence[i]);
    }

    clearSwapchainImageViews();
    m_vulkanDevice.m_logicalDevice.destroySwapchainKHR(m_swapchain);

    m_vulkanInstance.destroySurfaceKHR(m_renderingSurface);

    vmaDestroyAllocator(g_allocator);

    m_vulkanDevice.m_logicalDevice.destroy();

#if DEBUG || _DEBUG
    VulkanDebug::Get()->reset();
#endif
    m_vulkanInstance.destroy();
}

auto VulkanRenderer::create(uint32_t width, uint32_t height) -> void
{
    createInstance();
    createSurface();
    createDevice();
    createAllocators();
    createSyncObjects();

    onSize(width, height);
}

auto VulkanRenderer::onSize(uint32_t width, uint32_t height) -> void
{
    m_vulkanDevice.m_logicalDevice.waitIdle();

    clearSwapchainImageViews();
    querySwapchainCreateInfo(width, height);
    createSwapchain(width, height);
    for (const auto it : m_frameDependentObjects)
        it->recreate(m_swapchainCreateInfo.m_imageCount, width, height);
}

auto VulkanRenderer::acquire() -> void
{
    m_vulkanDevice.m_logicalDevice.waitForFences(1, &m_inFlightFence[m_inFlightFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    m_vulkanDevice.m_logicalDevice.resetFences(1, &m_inFlightFence[m_inFlightFrame]);
    vk::ResultValue<uint32_t> imageIndex = m_vulkanDevice.m_logicalDevice.acquireNextImageKHR(m_swapchain, std::numeric_limits<uint64_t>::max(),
        m_imageAvailableSemaphore[m_inFlightFrame], nullptr);
    switch (imageIndex.result)
    {
    case vk::Result::eSuccess:
        break;
    case vk::Result::eSuboptimalKHR:
    case vk::Result::eErrorOutOfDateKHR:
        onSize(m_swapchainCreateInfo.m_extent.width,
            m_swapchainCreateInfo.m_extent.height);
    default:
        THROW_ERROR("Something went wrong when acquiring image\n");
    }

    m_currentFrame = imageIndex.value;

    updateFrameDependentObjects(imageIndex.value);
}


auto VulkanRenderer::render(IGraphicsScene* scene) -> void
{
    auto commandBuffers = scene->getCommandBuffers(m_currentFrame);
    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBufferCount((uint32_t)commandBuffers.size());
    submitInfo.setPCommandBuffers(commandBuffers.data());
    submitInfo.setWaitSemaphoreCount(1);
    submitInfo.setPWaitSemaphores(&m_imageAvailableSemaphore[m_inFlightFrame]);
    submitInfo.setSignalSemaphoreCount(1);
    submitInfo.setPSignalSemaphores(&m_renderingFinishedSemaphore[m_inFlightFrame]);
    vk::PipelineStageFlags waitFlags[] = { vk::PipelineStageFlagBits::eTopOfPipe };
    submitInfo.setPWaitDstStageMask(waitFlags);
    m_vulkanDevice.m_queues.graphicsQueue.submit(1, &submitInfo, m_inFlightFence[m_inFlightFrame]);
}

auto VulkanRenderer::present() -> void
{
    vk::PresentInfoKHR presentInfo;
    presentInfo.setSwapchainCount(1);
    presentInfo.setPSwapchains(&m_swapchain);
    presentInfo.setWaitSemaphoreCount(1);
    presentInfo.setPWaitSemaphores(&m_renderingFinishedSemaphore[m_inFlightFrame]);
    presentInfo.setPImageIndices(&m_currentFrame);
    m_vulkanDevice.m_queues.presentQueue.presentKHR(presentInfo);

    m_inFlightFrame = (m_inFlightFrame + 1) % _maxInFlightFrames;
}

auto VulkanRenderer::wait() -> void
{
    m_vulkanDevice.m_logicalDevice.waitIdle();
}

auto VulkanRenderer::addFrameDependentObject(IFrameDependent* object) -> void
{
    object->create((uint32_t)m_swapchainInfo.m_imageViews.size(),
        m_swapchainCreateInfo.m_extent.width, m_swapchainCreateInfo.m_extent.height);
    m_frameDependentObjects.push_back(object);
}

auto VulkanRenderer::addInstanceLayer(const char * layer) -> bool
{
    bool res = false;
    for (uint32_t i = 0; i < m_presentLayers.size(); ++i)
    {
        if (strcmp(layer, m_presentLayers[i].layerName) == 0)
        {
            res = true;
            break;
        }
    }
    if (res)
        m_enabledLayers.push_back(layer);
    return res;
}

auto VulkanRenderer::addInstanceExtension(const char * extension) -> bool
{
    bool res = false;
    for (uint32_t i = 0; i < m_presentExtensions.size(); ++i)
    {
        if (strcmp(extension, m_presentExtensions[i].extensionName) == 0)
        {
            res = true;
            break;
        }
    }
    if (res)
        m_enabledExtensions.push_back(extension);
    return res;
}

auto VulkanRenderer::getPresentInstanceLayers() const -> std::vector<vk::LayerProperties>
{
    return m_presentLayers;
}

auto VulkanRenderer::getPresentInstanceExtensions() const -> std::vector<vk::ExtensionProperties>
{
    return m_presentExtensions;
}

auto VulkanRenderer::getVulkanInstance() const -> const vk::Instance&
{
    return m_vulkanInstance;
}

auto VulkanRenderer::getVulkanLogicalDevice() const -> const vk::Device &
{
    return m_vulkanDevice.m_logicalDevice;
}

auto VulkanRenderer::getVulkanDeviceInfo() const -> const DeviceInfo &
{
    return m_vulkanDevice;
}

auto VulkanRenderer::getVulkanSwapchainInfo() const -> const SwapchainInfo &
{
    return m_swapchainInfo;
}

auto VulkanRenderer::getVulkanSwapchainCreateInfo() const -> const SwapchainCreateInfo &
{
    return m_swapchainCreateInfo;
}

auto VulkanRenderer::createInstance() -> void
{
    vk::ApplicationInfo appInfo = {};
    appInfo.setApiVersion(VK_API_VERSION_1_1).setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
        .setEngineVersion(VK_MAKE_VERSION(1, 0, 0)).setPApplicationName(APPLICATION_NAME).setPEngineName(ENGINE_NAME);

    vk::InstanceCreateInfo instanceInfo;
    instanceInfo.setEnabledExtensionCount((uint32_t)m_enabledExtensions.size())
        .setPpEnabledExtensionNames(m_enabledExtensions.data())
        .setEnabledLayerCount((uint32_t)m_enabledLayers.size())
        .setPpEnabledLayerNames(m_enabledLayers.data())
        .setPApplicationInfo(&appInfo);

    EVALUATE(m_vulkanInstance = vk::createInstance(instanceInfo), nullptr, == , "Couldn't create a valid vulkan instance");

#if DEBUG || _DEBUG
    VulkanDebug::Get();
#endif
}

auto VulkanRenderer::createSurface() -> void
{
    EVALUATE(WindowObject::Get()->createVulkanSurface(m_vulkanInstance, m_renderingSurface), vk::Result::eSuccess,
        != , "Couldn't create vulkan surface");
}

auto VulkanRenderer::createDevice() -> void
{
    auto devices = m_vulkanInstance.enumeratePhysicalDevices();
    for (const auto& device : devices)
    {
        auto deviceLayers = device.enumerateDeviceLayerProperties();
        bool good = true;
        for (const auto& it : deviceEnabledLayers)
        {
            if (!CHECK_IF_STR_IN_ARRAY_COMPLEX(it, deviceLayers, layerName))
            {
                good = false;
                break;
            }
        }
        if (!good)
            continue;

        auto deviceExtensions = device.enumerateDeviceExtensionProperties();
        good = true;
        for (const auto& it : deviceEnabledExtensions)
        {
            deviceExtensions[0].extensionName;
            if (!CHECK_IF_STR_IN_ARRAY_COMPLEX(it, deviceExtensions, extensionName))
            {
                good = false;
                break;
            }
        }
        if (!good)
            continue;
        
        m_vulkanDevice.m_enabledFeatures = device.getFeatures();
        auto deviceProperties = device.getProperties();
        // auto deviceMemoryProperties = device.getMemoryProperties();
        m_vulkanDevice.m_physicalDevice = device;
        auto queueFamilies = m_vulkanDevice.m_physicalDevice.getQueueFamilyProperties();
        for (uint32_t i = 0; i < queueFamilies.size(); ++i)
        {
            vk::Bool32 canPresent = m_vulkanDevice.m_physicalDevice.getSurfaceSupportKHR(i, m_renderingSurface);
            if (canPresent && queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
            {
                m_vulkanDevice.m_families.presentIndex = i;
                m_vulkanDevice.m_families.graphicsIndex = i;
            }
            else if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
                m_vulkanDevice.m_families.graphicsIndex = i;
        }
        VkSampleCountFlags sampling = std::min(static_cast<VkSampleCountFlags>(deviceProperties.limits.framebufferColorSampleCounts),
            static_cast<VkSampleCountFlags>(deviceProperties.limits.framebufferDepthSampleCounts));

        if (sampling & VkSampleCountFlagBits::VK_SAMPLE_COUNT_64_BIT) { m_vulkanDevice.m_bestSampling = vk::SampleCountFlagBits::e64; }
        else if (sampling & VkSampleCountFlagBits::VK_SAMPLE_COUNT_32_BIT) { m_vulkanDevice.m_bestSampling = vk::SampleCountFlagBits::e32; }
        else if (sampling & VkSampleCountFlagBits::VK_SAMPLE_COUNT_16_BIT) { m_vulkanDevice.m_bestSampling = vk::SampleCountFlagBits::e16; }
        else if (sampling & VkSampleCountFlagBits::VK_SAMPLE_COUNT_8_BIT)  { m_vulkanDevice.m_bestSampling = vk::SampleCountFlagBits::e8; }
        else if (sampling & VkSampleCountFlagBits::VK_SAMPLE_COUNT_4_BIT)  { m_vulkanDevice.m_bestSampling = vk::SampleCountFlagBits::e4; }
        else if (sampling & VkSampleCountFlagBits::VK_SAMPLE_COUNT_2_BIT)  { m_vulkanDevice.m_bestSampling = vk::SampleCountFlagBits::e2; }
        else if (sampling & VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT)  { m_vulkanDevice.m_bestSampling = vk::SampleCountFlagBits::e1; }

        break;
    }
    
    EVALUATE(m_vulkanDevice.m_physicalDevice, nullptr, == , "Couldn't find a suitable device");

    std::unordered_set<decltype(m_vulkanDevice.m_families.graphicsIndex)> families{
        m_vulkanDevice.m_families.graphicsIndex,
        m_vulkanDevice.m_families.presentIndex
    };
    
    uint32_t numberOfIndices = (uint32_t)families.size();
    std::vector<vk::DeviceQueueCreateInfo> queues;
    queues.resize(numberOfIndices);
    for (uint32_t i = 0; i < numberOfIndices; ++i)
    {
        float priority = 1.0f;
        queues[i].setPQueuePriorities(&priority);
        queues[i].setQueueCount(1);
        queues[i].setQueueFamilyIndex(*((decltype(m_vulkanDevice.m_families.graphicsIndex)*)&m_vulkanDevice.m_families + i));
    }

    vk::DeviceCreateInfo deviceInfo = {};
    deviceInfo.setEnabledExtensionCount((uint32_t)deviceEnabledExtensions.size())
        .setPpEnabledExtensionNames(deviceEnabledExtensions.data())
        .setEnabledLayerCount((uint32_t)deviceEnabledLayers.size())
        .setPpEnabledLayerNames(deviceEnabledLayers.data())
        .setPQueueCreateInfos(queues.data())
        .setQueueCreateInfoCount(numberOfIndices)
        .setPEnabledFeatures(&m_vulkanDevice.m_enabledFeatures);
    EVALUATE(m_vulkanDevice.m_logicalDevice = m_vulkanDevice.m_physicalDevice.createDevice(deviceInfo), nullptr, == , "Couldn't create a logical device");


    m_vulkanDevice.m_queues.graphicsQueue = m_vulkanDevice.m_logicalDevice.getQueue(m_vulkanDevice.m_families.graphicsIndex, 0);
    m_vulkanDevice.m_queues.presentQueue = m_vulkanDevice.m_logicalDevice.getQueue(m_vulkanDevice.m_families.presentIndex, 0);
}

auto VulkanRenderer::createAllocators() -> void
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = (VkPhysicalDevice)m_vulkanDevice.m_physicalDevice;
    allocatorInfo.device = (VkDevice)m_vulkanDevice.m_logicalDevice;

    VkResult res = vmaCreateAllocator(&allocatorInfo, &g_allocator);
    EVALUATE(res, VkResult::VK_SUCCESS, != , "Couldn't create the allocator");
}

auto VulkanRenderer::createSwapchain(uint32_t width, uint32_t height) -> void
{
    vk::SwapchainCreateInfoKHR swapchainInfo;
    swapchainInfo.setClipped(VK_TRUE)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setImageArrayLayers(1)
        .setImageColorSpace(m_swapchainCreateInfo.m_format.colorSpace)
        .setImageFormat(m_swapchainCreateInfo.m_format.format)
        .setImageExtent(m_swapchainCreateInfo.m_extent)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setMinImageCount(m_swapchainCreateInfo.m_imageCount)
        .setPresentMode(m_swapchainCreateInfo.m_presentMode)
        .setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
        .setSurface(m_renderingSurface);

    if (m_vulkanDevice.m_families.graphicsIndex != m_vulkanDevice.m_families.presentIndex)
    {
        swapchainInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
            .setQueueFamilyIndexCount(2)
            .setPQueueFamilyIndices((uint32_t*)&m_vulkanDevice.m_families);
    }
    else
    {
        swapchainInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    }
    if (m_swapchain)
    {
        swapchainInfo.setOldSwapchain(m_swapchain);
    }

    vk::SwapchainKHR newSwapchain;
    newSwapchain = m_vulkanDevice.m_logicalDevice.createSwapchainKHR(swapchainInfo);
    EVALUATE(newSwapchain, nullptr, == , "Couldn't create swapchain");
    if (m_swapchain)
    {
        m_vulkanDevice.m_logicalDevice.destroySwapchainKHR(m_swapchain);
    }
    m_swapchain = newSwapchain;


    // Create image views for images
    m_swapchainInfo.m_images = m_vulkanDevice.m_logicalDevice.getSwapchainImagesKHR(m_swapchain);
    m_swapchainInfo.m_imageViews.reserve(m_swapchainInfo.m_images.size());
    for (const auto it : m_swapchainInfo.m_images)
    {
        vk::ImageSubresourceRange viewRange;
        viewRange.setBaseArrayLayer(0).setBaseMipLevel(0).setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setLayerCount(1).setLevelCount(1);
        vk::ComponentMapping componentMapping;
        componentMapping.setR(vk::ComponentSwizzle::eR).setG(vk::ComponentSwizzle::eG)
            .setB(vk::ComponentSwizzle::eB).setA(vk::ComponentSwizzle::eA);
        vk::ImageViewCreateInfo viewInfo;
        viewInfo.setFormat(m_swapchainCreateInfo.m_format.format)
            .setImage(it).setViewType(vk::ImageViewType::e2D)
            .setSubresourceRange(viewRange)
            .setComponents(componentMapping);
        vk::ImageView view;
        view = m_vulkanDevice.m_logicalDevice.createImageView(viewInfo);
        EVALUATE(view, nullptr, == , "Couldn't create a image view for a swapchain image");
        m_swapchainInfo.m_imageViews.push_back(view);
    }
}


auto VulkanRenderer::createSyncObjects() -> void
{
    vk::SemaphoreCreateInfo semaphoreInfo = {};
    vk::FenceCreateInfo fenceInfo = {};
    fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
    for (uint8_t i = 0; i < _maxInFlightFrames; ++i)
    {
        m_imageAvailableSemaphore[i] = m_vulkanDevice.m_logicalDevice.createSemaphore(semaphoreInfo);
        EVALUATE(m_imageAvailableSemaphore[i], nullptr, == , "Couldn't create %d image available semaphore", i);
        m_renderingFinishedSemaphore[i] = m_vulkanDevice.m_logicalDevice.createSemaphore(semaphoreInfo);
        EVALUATE(m_renderingFinishedSemaphore[i], nullptr, == , "Couldn't create %d rendering finished semaphore", i);
        m_inFlightFence[i] = m_vulkanDevice.m_logicalDevice.createFence(fenceInfo);
        EVALUATE(m_inFlightFence[i], nullptr, == , "Couldn't create %d in flight fence", i);
    }
}

auto VulkanRenderer::updateFrameDependentObjects(uint32_t currentImage) -> void
{
    for (const auto it : m_frameDependentObjects)
    {
        it->render(currentImage);
    }
}


auto VulkanRenderer::querySwapchainCreateInfo(uint32_t width, uint32_t height) -> void
{
    m_swapchainCapabilities.m_formats = m_vulkanDevice.m_physicalDevice.getSurfaceFormatsKHR(m_renderingSurface);
    m_swapchainCapabilities.m_presentModes = m_vulkanDevice.m_physicalDevice.getSurfacePresentModesKHR(m_renderingSurface);
    m_swapchainCapabilities.m_surfaceCapabilities = m_vulkanDevice.m_physicalDevice.getSurfaceCapabilitiesKHR(m_renderingSurface);

    m_swapchainCreateInfo.m_format = selectFormat();
    m_swapchainCreateInfo.m_presentMode = selectPresentMode();
    m_swapchainCreateInfo.m_extent = selectExtent(width, height);
}

auto VulkanRenderer::destroyUtilities() -> void
{
    OneTimeCommandBuffers::reset();
    Samplers::reset();
}

auto VulkanRenderer::selectExtent(uint32_t width, uint32_t height) -> vk::Extent2D
{
    vk::Extent2D result;
    result.width = std::max(
        m_swapchainCapabilities.m_surfaceCapabilities.minImageExtent.width,
        std::max(m_swapchainCapabilities.m_surfaceCapabilities.maxImageExtent.width,
            width));
    result.height = std::max(
        m_swapchainCapabilities.m_surfaceCapabilities.minImageExtent.height,
        std::max(m_swapchainCapabilities.m_surfaceCapabilities.maxImageExtent.height,
            height));
    return result;
}

auto VulkanRenderer::selectFormat() -> vk::SurfaceFormatKHR
{
    EVALUATE(m_swapchainCapabilities.m_formats.size(), 0, == , "Couldn't find available formats for your platform");
    for (const auto it : m_swapchainCapabilities.m_formats)
    {
        if (it.format == vk::Format::eR8G8B8A8Unorm && it.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            return it;
    }
    return m_swapchainCapabilities.m_formats[0];
}

auto VulkanRenderer::selectPresentMode() -> vk::PresentModeKHR
{
    m_swapchainCreateInfo.m_imageCount = m_swapchainCapabilities.m_surfaceCapabilities.minImageCount + 1;
    if (m_swapchainCreateInfo.m_imageCount > m_swapchainCapabilities.m_surfaceCapabilities.maxImageCount)
        m_swapchainCreateInfo.m_imageCount = m_swapchainCapabilities.m_surfaceCapabilities.minImageCount;

    if (!m_hasVsync)
        return vk::PresentModeKHR::eImmediate;

    vk::PresentModeKHR bestMode = vk::PresentModeKHR::eImmediate;

    for (const auto it : m_swapchainCapabilities.m_presentModes)
    {
        if (static_cast<int>(it) & static_cast<int>(vk::PresentModeKHR::eFifo))
            bestMode = it;
        else if (it == vk::PresentModeKHR::eMailbox)
            return it;
    }

    return bestMode;
}

auto VulkanRenderer::clearSwapchainImageViews() -> void
{
    for (const auto it : m_swapchainInfo.m_imageViews)
    {
        m_vulkanDevice.m_logicalDevice.destroyImageView(it);
    }
    m_swapchainInfo.m_imageViews.clear();
}
