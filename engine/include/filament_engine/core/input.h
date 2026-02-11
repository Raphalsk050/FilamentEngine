#pragma once

#include <filament_engine/math/types.h>

#include <array>
#include <cstdint>

namespace fe {

// Key codes (subset mapping to SDL scancodes)
enum class Key : int {
    Unknown = 0,
    A = 4, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num1 = 30, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, Num0,
    Return = 40,
    Escape = 41,
    Backspace = 42,
    Tab = 43,
    Space = 44,
    Right = 79,
    Left = 80,
    Down = 81,
    Up = 82,
    LCtrl = 224,
    LShift = 225,
    LAlt = 226,
    RCtrl = 228,
    RShift = 229,
    RAlt = 230,
    MaxKeys = 512
};

enum class MouseButton : int {
    Left = 1,
    Middle = 2,
    Right = 3,
    MaxButtons = 8
};

// Input state manager, updated each frame from SDL events
class Input {
public:
    // Called at the beginning of each frame to reset per-frame state
    void beginFrame();

    // Process an SDL key event
    void onKeyEvent(int scancode, bool pressed);

    // Process an SDL mouse motion event
    void onMouseMove(float x, float y, float dx, float dy);

    // Process an SDL mouse button event
    void onMouseButton(int button, bool pressed);

    // Process an SDL mouse scroll event
    void onMouseScroll(float dx, float dy);

    // Key queries
    bool isKeyDown(Key key) const;
    bool isKeyPressed(Key key) const;  // true only on the frame the key was pressed
    bool isKeyReleased(Key key) const; // true only on the frame the key was released

    // Mouse queries
    Vec2 getMousePosition() const { return m_mousePosition; }
    Vec2 getMouseDelta() const { return m_mouseDelta; }
    bool isMouseButtonDown(MouseButton button) const;
    bool isMouseButtonPressed(MouseButton button) const;
    bool isMouseButtonReleased(MouseButton button) const;
    Vec2 getScrollDelta() const { return m_scrollDelta; }

private:
    static constexpr int MAX_KEYS = static_cast<int>(Key::MaxKeys);
    static constexpr int MAX_BUTTONS = static_cast<int>(MouseButton::MaxButtons);

    std::array<bool, MAX_KEYS> m_keysDown{};
    std::array<bool, MAX_KEYS> m_keysPressed{};
    std::array<bool, MAX_KEYS> m_keysReleased{};

    std::array<bool, MAX_BUTTONS> m_mouseDown{};
    std::array<bool, MAX_BUTTONS> m_mousePressed{};
    std::array<bool, MAX_BUTTONS> m_mouseReleased{};

    Vec2 m_mousePosition{0, 0};
    Vec2 m_mouseDelta{0, 0};
    Vec2 m_scrollDelta{0, 0};
};

} // namespace fe
