#pragma once

#include <chrono>

class HighResolutionTimer
{
public:
    HighResolutionTimer(long long msPeriod = 1000);
    ~HighResolutionTimer();


    auto                                                    update() -> void;
    auto                                                    timeSinceStart() -> float;
    auto                                                    timeSincePeriodStart() -> float;
    auto                                                    timeSinceLastFrame() -> float;
    auto                                                    periodEnded() -> float;
    auto                                                    getPeriodCount() -> uint16_t;

private:
    std::chrono::time_point<std::chrono::system_clock>      m_startTimer;

    std::chrono::time_point<std::chrono::system_clock>      m_startPeriod;
    std::chrono::time_point<std::chrono::system_clock>      m_lastFrame;


    long long                                               m_duration;

    uint16_t                                                m_currentFPS;
    float                                                   m_frameTime;

    uint16_t                                                m_lastFPS;

    bool                                                    m_periodEnded;

};

