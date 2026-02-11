// Unit tests for Clock (frame timing)
#include <gtest/gtest.h>
#include <filament_engine/core/clock.h>

#include <thread>
#include <chrono>

// =====================
// Initial state
// =====================

TEST(Clock, InitialState_DeltaTimeZero) {
    fe::Clock clock;
    EXPECT_FLOAT_EQ(clock.getDeltaTime(), 0.0f);
}

TEST(Clock, InitialState_ElapsedTimeZero) {
    fe::Clock clock;
    EXPECT_DOUBLE_EQ(clock.getElapsedTime(), 0.0);
}

TEST(Clock, InitialState_FPS_Zero) {
    fe::Clock clock;
    // Before any tick, delta is 0 so FPS should be 0
    EXPECT_FLOAT_EQ(clock.getFPS(), 0.0f);
}

// =====================
// Tick behavior
// =====================

TEST(Clock, Tick_ProducesPositiveDeltaTime) {
    fe::Clock clock;
    // Small sleep to ensure measurable time passes
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    clock.tick();

    float dt = clock.getDeltaTime();
    EXPECT_GT(dt, 0.0f);
    // Should be roughly 10ms but give wide tolerance for CI
    EXPECT_LT(dt, 1.0f);
}

TEST(Clock, Tick_ElapsedTimeAccumulates) {
    fe::Clock clock;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    clock.tick();
    double elapsed1 = clock.getElapsedTime();
    EXPECT_GT(elapsed1, 0.0);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    clock.tick();
    double elapsed2 = clock.getElapsedTime();
    EXPECT_GT(elapsed2, elapsed1);
}

TEST(Clock, Tick_DeltaTimeUpdatesEachTick) {
    fe::Clock clock;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    clock.tick();
    float dt1 = clock.getDeltaTime();

    std::this_thread::sleep_for(std::chrono::milliseconds(15));
    clock.tick();
    float dt2 = clock.getDeltaTime();

    // Both should be positive
    EXPECT_GT(dt1, 0.0f);
    EXPECT_GT(dt2, 0.0f);
}

// =====================
// FPS calculation
// =====================

TEST(Clock, GetFPS_AfterTick_ReturnsPositive) {
    fe::Clock clock;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    clock.tick();

    float fps = clock.getFPS();
    EXPECT_GT(fps, 0.0f);
}

TEST(Clock, GetFPS_ConsistentWithDeltaTime) {
    fe::Clock clock;
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    clock.tick();

    float dt = clock.getDeltaTime();
    float fps = clock.getFPS();

    if (dt > 0.0f) {
        EXPECT_NEAR(fps, 1.0f / dt, 0.01f);
    }
}

// =====================
// Multiple ticks
// =====================

TEST(Clock, MultipleTicks_WorkCorrectly) {
    fe::Clock clock;
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        clock.tick();
        EXPECT_GT(clock.getDeltaTime(), 0.0f);
        EXPECT_GT(clock.getElapsedTime(), 0.0);
    }
}
