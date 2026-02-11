#include <filament_engine/core/clock.h>

namespace fe {

Clock::Clock()
    : m_startTime(std::chrono::high_resolution_clock::now())
    , m_lastTime(m_startTime) {
}

void Clock::tick() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<float>(now - m_lastTime);
    m_deltaTime = duration.count();
    m_elapsedTime = std::chrono::duration<double>(now - m_startTime).count();
    m_lastTime = now;
}

float Clock::getFPS() const {
    if (m_deltaTime > 0.0f) {
        return 1.0f / m_deltaTime;
    }
    return 0.0f;
}

} // namespace fe
