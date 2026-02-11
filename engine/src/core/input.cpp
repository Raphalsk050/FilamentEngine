#include <filament_engine/core/input.h>

#include <algorithm>

namespace fe {

void Input::beginFrame() {
    m_keysPressed.fill(false);
    m_keysReleased.fill(false);
    m_mousePressed.fill(false);
    m_mouseReleased.fill(false);
    m_mouseDelta = {0, 0};
    m_scrollDelta = {0, 0};
}

void Input::onKeyEvent(int scancode, bool pressed) {
    if (scancode < 0 || scancode >= MAX_KEYS) return;

    if (pressed && !m_keysDown[scancode]) {
        m_keysPressed[scancode] = true;
    }
    if (!pressed && m_keysDown[scancode]) {
        m_keysReleased[scancode] = true;
    }
    m_keysDown[scancode] = pressed;
}

void Input::onMouseMove(float x, float y, float dx, float dy) {
    m_mousePosition = {x, y};
    m_mouseDelta = {dx, dy};
}

void Input::onMouseButton(int button, bool pressed) {
    if (button < 0 || button >= MAX_BUTTONS) return;

    if (pressed && !m_mouseDown[button]) {
        m_mousePressed[button] = true;
    }
    if (!pressed && m_mouseDown[button]) {
        m_mouseReleased[button] = true;
    }
    m_mouseDown[button] = pressed;
}

void Input::onMouseScroll(float dx, float dy) {
    m_scrollDelta = {dx, dy};
}

bool Input::isKeyDown(Key key) const {
    int k = static_cast<int>(key);
    return (k >= 0 && k < MAX_KEYS) ? m_keysDown[k] : false;
}

bool Input::isKeyPressed(Key key) const {
    int k = static_cast<int>(key);
    return (k >= 0 && k < MAX_KEYS) ? m_keysPressed[k] : false;
}

bool Input::isKeyReleased(Key key) const {
    int k = static_cast<int>(key);
    return (k >= 0 && k < MAX_KEYS) ? m_keysReleased[k] : false;
}

bool Input::isMouseButtonDown(MouseButton button) const {
    int b = static_cast<int>(button);
    return (b >= 0 && b < MAX_BUTTONS) ? m_mouseDown[b] : false;
}

bool Input::isMouseButtonPressed(MouseButton button) const {
    int b = static_cast<int>(button);
    return (b >= 0 && b < MAX_BUTTONS) ? m_mousePressed[b] : false;
}

bool Input::isMouseButtonReleased(MouseButton button) const {
    int b = static_cast<int>(button);
    return (b >= 0 && b < MAX_BUTTONS) ? m_mouseReleased[b] : false;
}

} // namespace fe
