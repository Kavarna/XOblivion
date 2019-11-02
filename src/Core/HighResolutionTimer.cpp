#include "HighResolutionTimer.h"

HighResolutionTimer::HighResolutionTimer(long long duration) :
    m_duration(duration), m_currentFPS(0), m_frameTime(0.0f), m_lastFPS(0), m_periodEnded(false)
{
    m_startTimer = std::chrono::system_clock::now();
    m_lastFrame = std::chrono::system_clock::now();
    m_startPeriod = std::chrono::system_clock::now();
}

HighResolutionTimer::~HighResolutionTimer()
{
}

auto HighResolutionTimer::update() -> void
{
    auto now = std::chrono::system_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_startPeriod).count() >= m_duration)
    {
        m_lastFPS = m_currentFPS;
        m_currentFPS = 0;
        m_startPeriod = now;
        m_periodEnded = true;
    }
    else
    {
        m_periodEnded = false;
    }
    m_currentFPS++;
    m_frameTime = std::chrono::duration_cast<std::chrono::duration<float>>(now - m_lastFrame).count();
    m_lastFrame = now;
}

auto HighResolutionTimer::timeSinceStart() -> float
{
    return std::chrono::duration_cast<std::chrono::duration<float>>(m_startTimer - m_lastFrame).count();
}

auto HighResolutionTimer::timeSincePeriodStart() -> float
{
    return std::chrono::duration_cast<std::chrono::duration<float>>(m_startTimer - m_lastFrame).count();
}

auto HighResolutionTimer::timeSinceLastFrame() -> float
{
    return m_frameTime;
}

auto HighResolutionTimer::periodEnded() -> float
{
    return m_periodEnded;
}

auto HighResolutionTimer::getPeriodCount() -> uint16_t
{
    return m_lastFPS;
}
