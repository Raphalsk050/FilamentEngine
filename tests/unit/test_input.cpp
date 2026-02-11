// Unit tests for Input system
#include <gtest/gtest.h>
#include <filament_engine/core/input.h>

// =====================
// Default state
// =====================

TEST(Input, DefaultState_NoKeysPressed) {
    fe::Input input;
    EXPECT_FALSE(input.isKeyDown(fe::Key::W));
    EXPECT_FALSE(input.isKeyDown(fe::Key::A));
    EXPECT_FALSE(input.isKeyDown(fe::Key::S));
    EXPECT_FALSE(input.isKeyDown(fe::Key::D));
    EXPECT_FALSE(input.isKeyDown(fe::Key::Space));
    EXPECT_FALSE(input.isKeyDown(fe::Key::Escape));
}

TEST(Input, DefaultState_NoMouseButtons) {
    fe::Input input;
    EXPECT_FALSE(input.isMouseButtonDown(fe::MouseButton::Left));
    EXPECT_FALSE(input.isMouseButtonDown(fe::MouseButton::Right));
    EXPECT_FALSE(input.isMouseButtonDown(fe::MouseButton::Middle));
}

TEST(Input, DefaultState_MousePositionZero) {
    fe::Input input;
    auto pos = input.getMousePosition();
    EXPECT_FLOAT_EQ(pos.x, 0.0f);
    EXPECT_FLOAT_EQ(pos.y, 0.0f);
}

TEST(Input, DefaultState_DeltaZero) {
    fe::Input input;
    auto delta = input.getMouseDelta();
    EXPECT_FLOAT_EQ(delta.x, 0.0f);
    EXPECT_FLOAT_EQ(delta.y, 0.0f);
}

TEST(Input, DefaultState_ScrollDeltaZero) {
    fe::Input input;
    auto scroll = input.getScrollDelta();
    EXPECT_FLOAT_EQ(scroll.x, 0.0f);
    EXPECT_FLOAT_EQ(scroll.y, 0.0f);
}

// =====================
// Key events
// =====================

TEST(Input, KeyDown_ViaOnKeyEvent) {
    fe::Input input;
    input.onKeyEvent(static_cast<int>(fe::Key::W), true);
    EXPECT_TRUE(input.isKeyDown(fe::Key::W));
    EXPECT_FALSE(input.isKeyDown(fe::Key::S));
}

TEST(Input, KeyUp_ViaOnKeyEvent) {
    fe::Input input;
    input.onKeyEvent(static_cast<int>(fe::Key::W), true);
    EXPECT_TRUE(input.isKeyDown(fe::Key::W));
    input.onKeyEvent(static_cast<int>(fe::Key::W), false);
    EXPECT_FALSE(input.isKeyDown(fe::Key::W));
}

TEST(Input, KeyPressed_FrameTransition) {
    fe::Input input;
    input.onKeyEvent(static_cast<int>(fe::Key::Space), true);
    EXPECT_TRUE(input.isKeyPressed(fe::Key::Space));

    // After beginFrame, pressed should clear
    input.beginFrame();
    EXPECT_FALSE(input.isKeyPressed(fe::Key::Space));
    EXPECT_TRUE(input.isKeyDown(fe::Key::Space)); // still held
}

TEST(Input, KeyReleased_FrameTransition) {
    fe::Input input;
    input.onKeyEvent(static_cast<int>(fe::Key::Escape), true);
    input.beginFrame();

    input.onKeyEvent(static_cast<int>(fe::Key::Escape), false);
    EXPECT_TRUE(input.isKeyReleased(fe::Key::Escape));

    input.beginFrame();
    EXPECT_FALSE(input.isKeyReleased(fe::Key::Escape));
}

TEST(Input, MultipleKeys_Simultaneous) {
    fe::Input input;
    input.onKeyEvent(static_cast<int>(fe::Key::W), true);
    input.onKeyEvent(static_cast<int>(fe::Key::A), true);
    input.onKeyEvent(static_cast<int>(fe::Key::LShift), true);

    EXPECT_TRUE(input.isKeyDown(fe::Key::W));
    EXPECT_TRUE(input.isKeyDown(fe::Key::A));
    EXPECT_TRUE(input.isKeyDown(fe::Key::LShift));
    EXPECT_FALSE(input.isKeyDown(fe::Key::S));
}

TEST(Input, KeyPressed_NotTriggeredOnHold) {
    fe::Input input;
    input.onKeyEvent(static_cast<int>(fe::Key::W), true);
    EXPECT_TRUE(input.isKeyPressed(fe::Key::W));

    input.beginFrame();
    // Key is still held but should not register as "pressed" anymore
    EXPECT_FALSE(input.isKeyPressed(fe::Key::W));
    EXPECT_TRUE(input.isKeyDown(fe::Key::W));

    input.beginFrame();
    EXPECT_FALSE(input.isKeyPressed(fe::Key::W));
    EXPECT_TRUE(input.isKeyDown(fe::Key::W));
}

// =====================
// Mouse button events
// =====================

TEST(Input, MouseButtonDown) {
    fe::Input input;
    input.onMouseButton(static_cast<int>(fe::MouseButton::Right), true);
    EXPECT_TRUE(input.isMouseButtonDown(fe::MouseButton::Right));
    EXPECT_FALSE(input.isMouseButtonDown(fe::MouseButton::Left));
}

TEST(Input, MouseButtonPressed_FrameTransition) {
    fe::Input input;
    input.onMouseButton(static_cast<int>(fe::MouseButton::Left), true);
    EXPECT_TRUE(input.isMouseButtonPressed(fe::MouseButton::Left));

    input.beginFrame();
    EXPECT_FALSE(input.isMouseButtonPressed(fe::MouseButton::Left));
    EXPECT_TRUE(input.isMouseButtonDown(fe::MouseButton::Left));
}

TEST(Input, MouseButtonReleased_FrameTransition) {
    fe::Input input;
    input.onMouseButton(static_cast<int>(fe::MouseButton::Left), true);
    input.beginFrame();

    input.onMouseButton(static_cast<int>(fe::MouseButton::Left), false);
    EXPECT_TRUE(input.isMouseButtonReleased(fe::MouseButton::Left));

    input.beginFrame();
    EXPECT_FALSE(input.isMouseButtonReleased(fe::MouseButton::Left));
}

// =====================
// Mouse movement
// =====================

TEST(Input, MouseMove_UpdatesPosition) {
    fe::Input input;
    input.onMouseMove(100.0f, 200.0f, 5.0f, -3.0f);
    auto pos = input.getMousePosition();
    EXPECT_FLOAT_EQ(pos.x, 100.0f);
    EXPECT_FLOAT_EQ(pos.y, 200.0f);
    auto delta = input.getMouseDelta();
    EXPECT_FLOAT_EQ(delta.x, 5.0f);
    EXPECT_FLOAT_EQ(delta.y, -3.0f);
}

TEST(Input, MouseMove_PositionPersistsAfterBeginFrame) {
    fe::Input input;
    input.onMouseMove(100.0f, 200.0f, 5.0f, -3.0f);
    input.beginFrame();

    auto pos = input.getMousePosition();
    EXPECT_FLOAT_EQ(pos.x, 100.0f);
    EXPECT_FLOAT_EQ(pos.y, 200.0f);
}

// =====================
// Mouse scroll
// =====================

TEST(Input, MouseScroll) {
    fe::Input input;
    input.onMouseScroll(0.0f, 3.0f);
    auto scroll = input.getScrollDelta();
    EXPECT_FLOAT_EQ(scroll.x, 0.0f);
    EXPECT_FLOAT_EQ(scroll.y, 3.0f);
}

// =====================
// Frame reset
// =====================

TEST(Input, BeginFrame_ClearsDeltas) {
    fe::Input input;
    input.onMouseMove(100.0f, 200.0f, 5.0f, -3.0f);
    input.onMouseScroll(1.0f, 2.0f);
    input.beginFrame();

    auto delta = input.getMouseDelta();
    EXPECT_FLOAT_EQ(delta.x, 0.0f);
    EXPECT_FLOAT_EQ(delta.y, 0.0f);

    auto scroll = input.getScrollDelta();
    EXPECT_FLOAT_EQ(scroll.x, 0.0f);
    EXPECT_FLOAT_EQ(scroll.y, 0.0f);
}

TEST(Input, BeginFrame_ClearsAllPerFrameState) {
    fe::Input input;
    // Press key, button, move, scroll
    input.onKeyEvent(static_cast<int>(fe::Key::W), true);
    input.onMouseButton(static_cast<int>(fe::MouseButton::Left), true);
    input.onMouseMove(50.0f, 60.0f, 10.0f, 20.0f);
    input.onMouseScroll(1.0f, -1.0f);

    input.beginFrame();

    // Per-frame state should be cleared
    EXPECT_FALSE(input.isKeyPressed(fe::Key::W));
    EXPECT_FALSE(input.isMouseButtonPressed(fe::MouseButton::Left));
    EXPECT_FLOAT_EQ(input.getMouseDelta().x, 0.0f);
    EXPECT_FLOAT_EQ(input.getScrollDelta().y, 0.0f);

    // Persistent state should remain
    EXPECT_TRUE(input.isKeyDown(fe::Key::W));
    EXPECT_TRUE(input.isMouseButtonDown(fe::MouseButton::Left));
    EXPECT_FLOAT_EQ(input.getMousePosition().x, 50.0f);
}
