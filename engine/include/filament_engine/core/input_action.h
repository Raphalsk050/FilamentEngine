#pragma once

#include <filament_engine/core/input.h>
#include <filament_engine/math/types.h>

#include <string>
#include <vector>

namespace fe {

// Type of input action: digital (bool), single axis (float), or dual axis (Vec2)
enum class InputActionType {
    Digital, // On/Off — e.g. "Jump", "Fire"
    Axis1D,  // Scalar — e.g. "MoveForward" (W=+1, S=-1)
    Axis2D   // 2D vector — e.g. "Look" (mouse delta)
};

// Source of an input binding
enum class InputSource {
    Key,         // Keyboard key
    MouseButton, // Mouse button
    MouseAxisX,  // Mouse X movement delta
    MouseAxisY,  // Mouse Y movement delta
    ScrollX,     // Mouse scroll X
    ScrollY      // Mouse scroll Y
};

// A single binding that maps a physical input to an action
struct InputBinding {
    InputSource source = InputSource::Key;
    Key key = Key::Unknown;
    MouseButton mouseButton = MouseButton::Left;

    // Scale factor for axis contribution (e.g. -1.0 for inverted axis, +1.0 for normal)
    float scale = 1.0f;

    // For Axis2D: which component this binding contributes to (0 = X, 1 = Y)
    int axisIndex = 0;
};

// Per-frame state of an input action
struct InputActionState {
    float value = 0.0f;     // For Axis1D: scalar value; For Digital: 1.0 or 0.0
    Vec2 axis2D{0, 0};      // For Axis2D: combined 2D value
    bool held = false;       // True while the action is active
    bool pressed = false;    // True only on the frame the action was first activated
    bool released = false;   // True only on the frame the action was deactivated
};

// A named input action with one or more bindings
class InputAction {
public:
    InputAction() = default;
    InputAction(std::string name, InputActionType type);

    // Evaluate the current state from raw input
    void evaluate(const Input& input);

    // Called at the start of each frame to prepare per-frame flags
    void beginFrame();

    const std::string& getName() const { return m_name; }
    InputActionType getType() const { return m_type; }
    const InputActionState& getState() const { return m_state; }

    // Binding management
    void addBinding(InputBinding binding);
    const std::vector<InputBinding>& getBindings() const { return m_bindings; }
    void clearBindings();

private:
    std::string m_name;
    InputActionType m_type = InputActionType::Digital;
    std::vector<InputBinding> m_bindings;
    InputActionState m_state;
    bool m_wasHeld = false; // previous frame's held state for pressed/released detection
};

} // namespace fe
