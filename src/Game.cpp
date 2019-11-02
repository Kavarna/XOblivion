#include "Game.h"
#include "Core/Window.h"

#include "Graphics/VulkanRenderer.h"

Game::Game()
{
    InitCore();
    InitGraphics();
    InitScenes();
}

Game::~Game()
{
    VulkanRenderer::Get()->wait();
    DeinitScenes();
    VulkanRenderer::Get()->reset();
    WindowObject::Get()->reset();
}

void Game::update()
{
    m_timer->update();
    m_simpleScene->update(m_timer->timeSinceLastFrame());
    if (Input::Get()->getKeyState("ESCAPE") == Input::EKeyState::ePress)
    {
        WindowObject::Get()->toggleMouse();
    }

    WindowObject::Get()->setWindowTitle(appendToString("Vulkan renderer: FPS: ", m_timer->getPeriodCount(), "; delta: ", m_timer->timeSinceLastFrame()));

}

void Game::render()
{
    VulkanRenderer::Get()->acquire();
    VulkanRenderer::Get()->render(m_simpleScene.get());
    VulkanRenderer::Get()->present();
}

void Game::onSize(uint32_t width, uint32_t height)
{
    VulkanRenderer::Get()->onSize(width, height);
}

void Game::run()
{
    WindowObject::Get()->run();
}


void Game::InitCore()
{
    WindowObject::Get(width, height);


    WindowObject::Get()->setUpdateCallback(std::bind(&Game::update, this));
    WindowObject::Get()->setRenderCallback(std::bind(&Game::render, this));
    WindowObject::Get()->setResizeCallback(std::bind(&Game::onSize, this,
        std::placeholders::_1, std::placeholders::_2));

    m_timer = std::make_unique<HighResolutionTimer>();
}

void Game::InitGraphics()
{
    InitVulkan();
}

void Game::InitScenes()
{
    m_simpleScene = std::make_unique<SimpleScene>();
    VulkanRenderer::Get()->addFrameDependentObject(m_simpleScene.get());
}

void Game::InitVulkan()
{
    auto layers = VulkanRenderer::Get()->getPresentInstanceLayers();
    auto extensions = VulkanRenderer::Get()->getPresentInstanceExtensions();

#if DEBUG || _DEBUG
    VulkanRenderer::Get()->addInstanceLayer("VK_LAYER_LUNARG_standard_validation");
    VulkanRenderer::Get()->addInstanceExtension("VK_EXT_debug_report");
#endif

    auto requiredExtensions = WindowObject::Get()->getWindowExtensions();

    for (auto extension : requiredExtensions)
    {
        VulkanRenderer::Get()->addInstanceExtension(extension);
    }

    VulkanRenderer::Get()->create(width, height);

}

void Game::DeinitScenes()
{
    m_simpleScene.reset();
}


