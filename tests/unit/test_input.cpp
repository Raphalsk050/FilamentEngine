// Unit tests for Input system
#include "../test_helpers.h"
#include <filament_engine/core/input.h>

TEST(Input_DefaultState_NoKeysPressed) {
    fe::Input input;
    ASSERT_FALSE(input.isKeyDown(fe::Key::W));
    ASSERT_FALSE(input.isKeyDown(fe::Key::A));
    ASSERT_FALSE(input.isKeyDown(fe::Key::S));
    ASSERT_FALSE(input.isKeyDown(fe::Key::D));
    ASSERT_FALSE(input.isKeyDown(fe::Key::Space));
    ASSERT_FALSE(input.isKeyDown(fe::Key::Escape));
}

TEST(Input_DefaultState_NoMouseButtons) {
    fe::Input input;
    ASSERT_FALSE(input.isMouseButtonDown(fe::MouseButton::Left));
    ASSERT_FALSE(input.isMouseButtonDown(fe::MouseButton::Right));
    ASSERT_FALSE(input.isMouseButtonDown(fe::MouseButton::Middle));
}

TEST(Input_DefaultState_MousePositionZero) {
    fe::Input input;
    auto pos = input.getMousePosition();
    ASSERT_NEAR(pos.x, 0.0f, 1e-6f);
    ASSERT_NEAR(pos.y, 0.0f, 1e-6f);
}

TEST(Input_DefaultState_DeltaZero) {
    fe::Input input;
    auto delta = input.getMouseDelta();
    ASSERT_NEAR(delta.x, 0.0f, 1e-6f);
    ASSERT_NEAR(delta.y, 0.0f, 1e-6f);
}

TEST(Input_DefaultState_ScrollDeltaZero) {
    fe::Input input;
    auto scroll = input.getScrollDelta();
    ASSERT_NEAR(scroll.x, 0.0f, 1e-6f);
    ASSERT_NEAR(scroll.y, 0.0f, 1e-6f);
}

TEST(Input_KeyDown_ViaOnKeyEvent) {
    fe::Input input;
    input.onKeyEvent(static_cast<int>(fe::Key::W), true);
    ASSERT_TRUE(input.isKeyDown(fe::Key::W));
    ASSERT_FALSE(input.isKeyDown(fe::Key::S)); // other keys unaffected
}

TEST(Input_KeyUp_ViaOnKeyEvent) {
    fe::Input input;
    input.onKeyEvent(static_cast<int>(fe::Key::W), true);
    ASSERT_TRUE(input.isKeyDown(fe::Key::W));
    input.onKeyEvent(static_cast<int>(fe::Key::W), false);
    ASSERT_FALSE(input.isKeyDown(fe::Key::W));
}

TEST(Input_KeyPressed_FrameTransition) {
    fe::Input input;
    // First frame: key goes down
    input.onKeyEvent(static_cast<int>(fe::Key::Space), true);
    ASSERT_TRUE(input.isKeyPressed(fe::Key::Space));

    // After beginFrame, pressed should clear
    input.beginFrame();
    ASSERT_FALSE(input.isKeyPressed(fe::Key::Space));
    ASSERT_TRUE(input.isKeyDown(fe::Key::Space)); // still held
}

TEST(Input_KeyReleased_FrameTransition) {
    fe::Input input;
    input.onKeyEvent(static_cast<int>(fe::Key::Escape), true);
    input.beginFrame(); // clear pressed flags

    // Release key
    input.onKeyEvent(static_cast<int>(fe::Key::Escape), false);
    ASSERT_TRUE(input.isKeyReleased(fe::Key::Escape));

    // After beginFrame, released should clear
    input.beginFrame();
    ASSERT_FALSE(input.isKeyReleased(fe::Key::Escape));
}

TEST(Input_MouseButtonDown) {
    fe::Input input;
    input.onMouseButton(static_cast<int>(fe::MouseButton::Right), true);
    ASSERT_TRUE(input.isMouseButtonDown(fe::MouseButton::Right));
    ASSERT_FALSE(input.isMouseButtonDown(fe::MouseButton::Left));
}

TEST(Input_MouseMove_UpdatesPosition) {
    fe::Input input;
    input.onMouseMove(100.0f, 200.0f, 5.0f, -3.0f);
    auto pos = input.getMousePosition();
    ASSERT_NEAR(pos.x, 100.0f, 1e-6f);
    ASSERT_NEAR(pos.y, 200.0f, 1e-6f);
    auto delta = input.getMouseDelta();
    ASSERT_NEAR(delta.x, 5.0f, 1e-6f);
    ASSERT_NEAR(delta.y, -3.0f, 1e-6f);
}

TEST(Input_MouseScroll) {
    fe::Input input;
    input.onMouseScroll(0.0f, 3.0f);
    auto scroll = input.getScrollDelta();
    ASSERT_NEAR(scroll.x, 0.0f, 1e-6f);
    ASSERT_NEAR(scroll.y, 3.0f, 1e-6f);
}

TEST(Input_BeginFrame_ClearsDeltas) {
    fe::Input input;
    input.onMouseMove(100.0f, 200.0f, 5.0f, -3.0f);
    input.onMouseScroll(1.0f, 2.0f);
    input.beginFrame();
    auto delta = input.getMouseDelta();
    ASSERT_NEAR(delta.x, 0.0f, 1e-6f);
    ASSERT_NEAR(delta.y, 0.0f, 1e-6f);
    auto scroll = input.getScrollDelta();
    ASSERT_NEAR(scroll.x, 0.0f, 1e-6f);
    ASSERT_NEAR(scroll.y, 0.0f, 1e-6f);
}

int main() {
    return runAllTests();
}
