#pragma once

#include "Oblivion.h"

#include "Core/HighResolutionTimer.h"
#include "Scenes/SimpleScene.h"

class Game : public ISingletone<Game>
{
    static constexpr const uint32_t width = 800;
    static constexpr const uint32_t height = 600;
public:
    Game();
    ~Game();

public:
    void run();

private:
    void update();
    void render();
    void onSize(uint32_t, uint32_t);

private:
    void InitCore();
    void InitGraphics();
    void InitScenes();

private:
    void InitVulkan();

private:
    void DeinitScenes();

private:
    std::unique_ptr<SimpleScene>            m_simpleScene;
    std::unique_ptr<HighResolutionTimer>    m_timer;

};