#pragma once

#include <chrono>

namespace fe {

// High-resolution clock for frame timing
class Clock {
public:
    Clock();

    // Call once per frame to update delta time
    void tick();

    // Time elapsed since last tick (seconds)
    float getDeltaTime() const { return m_deltaTime; }

    // Total time elapsed since clock creation (seconds)
    double getElapsedTime() const { return m_elapsedTime; }

    // Approximate frames per second
    float getFPS() const;

private:
    using TimePoint = std::chrono::high_resolution_clock::time_point;

    TimePoint m_startTime;
    TimePoint m_lastTime;
    float m_deltaTime = 0.0f;
    double m_elapsedTime = 0.0;
};

} // namespace fe
