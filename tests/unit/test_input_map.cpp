// Unit tests for InputMap
#include <gtest/gtest.h>
#include <filament_engine/core/input_map.h>
#include <filament_engine/core/input.h>

// ==============================
// InputMap — Creation and lookup
// ==============================

TEST(InputMap, CreateAction) {
    fe::InputMap map("Test");
    auto& action = map.createAction("Fire", fe::InputActionType::Digital);
    EXPECT_EQ(action.getName(), "Fire");
    EXPECT_EQ(map.getActionCount(), 1u);
}

TEST(InputMap, HasAction) {
    fe::InputMap map;
    EXPECT_FALSE(map.hasAction("Fire"));
    map.createAction("Fire", fe::InputActionType::Digital);
    EXPECT_TRUE(map.hasAction("Fire"));
}

TEST(InputMap, GetAction_ReturnsNullForMissing) {
    fe::InputMap map;
    EXPECT_EQ(map.getAction("Nope"), nullptr);
}

TEST(InputMap, GetAction_ValidAction) {
    fe::InputMap map;
    map.createAction("Jump", fe::InputActionType::Digital);
    auto* action = map.getAction("Jump");
    ASSERT_NE(action, nullptr);
    EXPECT_EQ(action->getName(), "Jump");
}

TEST(InputMap, RemoveAction) {
    fe::InputMap map;
    map.createAction("Fire", fe::InputActionType::Digital);
    EXPECT_TRUE(map.hasAction("Fire"));
    map.removeAction("Fire");
    EXPECT_FALSE(map.hasAction("Fire"));
    EXPECT_EQ(map.getActionCount(), 0u);
}

TEST(InputMap, DuplicateCreate_ReturnsSameAction) {
    fe::InputMap map;
    auto& a1 = map.createAction("Fire", fe::InputActionType::Digital);
    auto& a2 = map.createAction("Fire", fe::InputActionType::Digital);
    EXPECT_EQ(&a1, &a2);
    EXPECT_EQ(map.getActionCount(), 1u);
}

// ==============================
// InputMap — Bindings
// ==============================

TEST(InputMap, AddBinding_ValidAction) {
    fe::InputMap map;
    map.createAction("Fire", fe::InputActionType::Digital);
    map.addBinding("Fire", {fe::InputSource::Key, fe::Key::Space});

    auto* action = map.getAction("Fire");
    ASSERT_NE(action, nullptr);
    EXPECT_EQ(action->getBindings().size(), 1u);
}

TEST(InputMap, AddBinding_InvalidAction_NoOp) {
    fe::InputMap map;
    // Should not crash
    map.addBinding("Nonexistent", {fe::InputSource::Key, fe::Key::Space});
    EXPECT_EQ(map.getActionCount(), 0u);
}

// ==============================
// InputMap — Queries after update
// ==============================

TEST(InputMap, IsHeld_WithKeyDown) {
    fe::InputMap map;
    map.createAction("Fire", fe::InputActionType::Digital);
    map.addBinding("Fire", {fe::InputSource::Key, fe::Key::Space});

    fe::Input input;
    input.onKeyEvent(static_cast<int>(fe::Key::Space), true);
    map.update(input);

    EXPECT_TRUE(map.isHeld("Fire"));
    EXPECT_TRUE(map.isPressed("Fire"));
}

TEST(InputMap, IsPressed_OnlyFirstFrame) {
    fe::InputMap map;
    map.createAction("Fire", fe::InputActionType::Digital);
    map.addBinding("Fire", {fe::InputSource::Key, fe::Key::Space});

    fe::Input input;
    input.onKeyEvent(static_cast<int>(fe::Key::Space), true);

    // Frame 1
    map.update(input);
    EXPECT_TRUE(map.isPressed("Fire"));

    // Frame 2 — still held but no longer pressed
    map.update(input);
    EXPECT_FALSE(map.isPressed("Fire"));
    EXPECT_TRUE(map.isHeld("Fire"));
}

TEST(InputMap, IsReleased_AfterKeyUp) {
    fe::InputMap map;
    map.createAction("Fire", fe::InputActionType::Digital);
    map.addBinding("Fire", {fe::InputSource::Key, fe::Key::Space});

    fe::Input input;
    input.onKeyEvent(static_cast<int>(fe::Key::Space), true);
    map.update(input);

    // Release key
    input.onKeyEvent(static_cast<int>(fe::Key::Space), false);
    map.update(input);

    EXPECT_TRUE(map.isReleased("Fire"));
    EXPECT_FALSE(map.isHeld("Fire"));
}

TEST(InputMap, GetAxis_Composite) {
    fe::InputMap map;
    map.createAction("Move", fe::InputActionType::Axis1D);

    fe::InputBinding fwd;
    fwd.source = fe::InputSource::Key;
    fwd.key = fe::Key::W;
    fwd.scale = 1.0f;
    map.addBinding("Move", fwd);

    fe::InputBinding back;
    back.source = fe::InputSource::Key;
    back.key = fe::Key::S;
    back.scale = -1.0f;
    map.addBinding("Move", back);

    fe::Input input;
    input.onKeyEvent(static_cast<int>(fe::Key::W), true);
    map.update(input);

    EXPECT_FLOAT_EQ(map.getAxis("Move"), 1.0f);
}

TEST(InputMap, GetAxis2D_WASD) {
    fe::InputMap map;
    map.createAction("Move2D", fe::InputActionType::Axis2D);

    fe::InputBinding right;
    right.source = fe::InputSource::Key;
    right.key = fe::Key::D;
    right.scale = 1.0f;
    right.axisIndex = 0;
    map.addBinding("Move2D", right);

    fe::InputBinding up;
    up.source = fe::InputSource::Key;
    up.key = fe::Key::W;
    up.scale = 1.0f;
    up.axisIndex = 1;
    map.addBinding("Move2D", up);

    fe::Input input;
    input.onKeyEvent(static_cast<int>(fe::Key::D), true);
    input.onKeyEvent(static_cast<int>(fe::Key::W), true);
    map.update(input);

    auto axis = map.getAxis2D("Move2D");
    EXPECT_FLOAT_EQ(axis.x, 1.0f);
    EXPECT_FLOAT_EQ(axis.y, 1.0f);
}

// Missing action queries return safe defaults
TEST(InputMap, Query_NonexistentAction_SafeDefaults) {
    fe::InputMap map;
    EXPECT_FALSE(map.isHeld("Nope"));
    EXPECT_FALSE(map.isPressed("Nope"));
    EXPECT_FALSE(map.isReleased("Nope"));
    EXPECT_FLOAT_EQ(map.getAxis("Nope"), 0.0f);
    auto axis = map.getAxis2D("Nope");
    EXPECT_FLOAT_EQ(axis.x, 0.0f);
    EXPECT_FLOAT_EQ(axis.y, 0.0f);
}

TEST(InputMap, MapName) {
    fe::InputMap map("Gameplay");
    EXPECT_EQ(map.getName(), "Gameplay");
}
