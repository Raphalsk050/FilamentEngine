// Unit tests for InputAction
#include <gtest/gtest.h>
#include <filament_engine/core/input_action.h>
#include <filament_engine/core/input.h>

// ==============================
// InputAction — Digital type
// ==============================

TEST(InputAction, Digital_DefaultState) {
    fe::InputAction action("Fire", fe::InputActionType::Digital);
    auto& state = action.getState();
    EXPECT_FALSE(state.held);
    EXPECT_FALSE(state.pressed);
    EXPECT_FALSE(state.released);
    EXPECT_FLOAT_EQ(state.value, 0.0f);
}

TEST(InputAction, Digital_KeyBinding_Pressed) {
    fe::Input input;
    fe::InputAction action("Fire", fe::InputActionType::Digital);
    action.addBinding({fe::InputSource::Key, fe::Key::Space});

    // Simulate key press
    input.onKeyEvent(static_cast<int>(fe::Key::Space), true);

    action.beginFrame();
    action.evaluate(input);

    EXPECT_TRUE(action.getState().held);
    EXPECT_TRUE(action.getState().pressed);
    EXPECT_FALSE(action.getState().released);
    EXPECT_FLOAT_EQ(action.getState().value, 1.0f);
}

TEST(InputAction, Digital_KeyBinding_Released) {
    fe::Input input;
    fe::InputAction action("Fire", fe::InputActionType::Digital);
    action.addBinding({fe::InputSource::Key, fe::Key::Space});

    // Frame 1: press
    input.onKeyEvent(static_cast<int>(fe::Key::Space), true);
    action.beginFrame();
    action.evaluate(input);

    // Frame 2: release
    input.onKeyEvent(static_cast<int>(fe::Key::Space), false);
    action.beginFrame();
    action.evaluate(input);

    EXPECT_FALSE(action.getState().held);
    EXPECT_FALSE(action.getState().pressed);
    EXPECT_TRUE(action.getState().released);
}

TEST(InputAction, Digital_MultipleBindings) {
    fe::Input input;
    fe::InputAction action("Fire", fe::InputActionType::Digital);
    action.addBinding({fe::InputSource::Key, fe::Key::Space});
    action.addBinding({fe::InputSource::MouseButton, fe::Key::Unknown, fe::MouseButton::Left});

    // Only mouse button pressed
    input.onMouseButton(static_cast<int>(fe::MouseButton::Left), true);
    action.beginFrame();
    action.evaluate(input);

    EXPECT_TRUE(action.getState().held);
    EXPECT_TRUE(action.getState().pressed);
}

TEST(InputAction, Digital_HeldNotPressed_OnSubsequentFrames) {
    fe::Input input;
    fe::InputAction action("Fire", fe::InputActionType::Digital);
    action.addBinding({fe::InputSource::Key, fe::Key::Space});

    input.onKeyEvent(static_cast<int>(fe::Key::Space), true);

    // Frame 1: pressed + held
    action.beginFrame();
    action.evaluate(input);
    EXPECT_TRUE(action.getState().pressed);
    EXPECT_TRUE(action.getState().held);

    // Frame 2: held but not pressed
    action.beginFrame();
    action.evaluate(input);
    EXPECT_FALSE(action.getState().pressed);
    EXPECT_TRUE(action.getState().held);
}

// ==============================
// InputAction — Axis1D type
// ==============================

TEST(InputAction, Axis1D_TwoKeys_Composite) {
    fe::Input input;
    fe::InputAction action("MoveForward", fe::InputActionType::Axis1D);

    fe::InputBinding forward;
    forward.source = fe::InputSource::Key;
    forward.key = fe::Key::W;
    forward.scale = 1.0f;
    action.addBinding(forward);

    fe::InputBinding backward;
    backward.source = fe::InputSource::Key;
    backward.key = fe::Key::S;
    backward.scale = -1.0f;
    action.addBinding(backward);

    // Press W only
    input.onKeyEvent(static_cast<int>(fe::Key::W), true);
    action.beginFrame();
    action.evaluate(input);
    EXPECT_FLOAT_EQ(action.getState().value, 1.0f);

    // Press W + S (should cancel out)
    input.onKeyEvent(static_cast<int>(fe::Key::S), true);
    action.beginFrame();
    action.evaluate(input);
    EXPECT_FLOAT_EQ(action.getState().value, 0.0f);

    // Release W, only S (should be -1)
    input.onKeyEvent(static_cast<int>(fe::Key::W), false);
    action.beginFrame();
    action.evaluate(input);
    EXPECT_FLOAT_EQ(action.getState().value, -1.0f);
}

TEST(InputAction, Axis1D_MouseAxis) {
    fe::Input input;
    fe::InputAction action("LookX", fe::InputActionType::Axis1D);

    fe::InputBinding mouseX;
    mouseX.source = fe::InputSource::MouseAxisX;
    mouseX.scale = 0.5f;
    action.addBinding(mouseX);

    input.onMouseMove(100, 200, 10.0f, 5.0f);
    action.beginFrame();
    action.evaluate(input);

    // 10.0 * 0.5 = 5.0
    EXPECT_FLOAT_EQ(action.getState().value, 5.0f);
}

// ==============================
// InputAction — Axis2D type
// ==============================

TEST(InputAction, Axis2D_WASD) {
    fe::Input input;
    fe::InputAction action("Move", fe::InputActionType::Axis2D);

    // X axis: A(-1) / D(+1)
    fe::InputBinding left;
    left.source = fe::InputSource::Key;
    left.key = fe::Key::A;
    left.scale = -1.0f;
    left.axisIndex = 0;
    action.addBinding(left);

    fe::InputBinding right;
    right.source = fe::InputSource::Key;
    right.key = fe::Key::D;
    right.scale = 1.0f;
    right.axisIndex = 0;
    action.addBinding(right);

    // Y axis: W(+1) / S(-1)
    fe::InputBinding fwd;
    fwd.source = fe::InputSource::Key;
    fwd.key = fe::Key::W;
    fwd.scale = 1.0f;
    fwd.axisIndex = 1;
    action.addBinding(fwd);

    fe::InputBinding back;
    back.source = fe::InputSource::Key;
    back.key = fe::Key::S;
    back.scale = -1.0f;
    back.axisIndex = 1;
    action.addBinding(back);

    // Press W + D => (1, 1)
    input.onKeyEvent(static_cast<int>(fe::Key::W), true);
    input.onKeyEvent(static_cast<int>(fe::Key::D), true);
    action.beginFrame();
    action.evaluate(input);

    EXPECT_FLOAT_EQ(action.getState().axis2D.x, 1.0f);
    EXPECT_FLOAT_EQ(action.getState().axis2D.y, 1.0f);
}

// ==============================
// InputAction — Binding management
// ==============================

TEST(InputAction, ClearBindings) {
    fe::InputAction action("Test", fe::InputActionType::Digital);
    action.addBinding({fe::InputSource::Key, fe::Key::W});
    EXPECT_EQ(action.getBindings().size(), 1u);

    action.clearBindings();
    EXPECT_TRUE(action.getBindings().empty());
}

TEST(InputAction, NameAndType) {
    fe::InputAction action("Jump", fe::InputActionType::Digital);
    EXPECT_EQ(action.getName(), "Jump");
    EXPECT_EQ(action.getType(), fe::InputActionType::Digital);
}
