#pragma once

#include <filament_engine/core/input_action.h>
#include <filament_engine/core/input.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace fe {

// InputMap — a named collection of InputActions.
// Provides a high-level API for defining and querying abstract input actions.
// Multiple InputMaps can coexist (e.g. one for gameplay, one for editor).
class InputMap {
public:
    InputMap() = default;
    explicit InputMap(std::string name);

    // Create a new action and return a reference for further configuration
    InputAction& createAction(const std::string& actionName, InputActionType type);

    // Add a binding to an existing action
    void addBinding(const std::string& actionName, InputBinding binding);

    // Remove an action entirely
    void removeAction(const std::string& actionName);

    // Query whether an action exists
    bool hasAction(const std::string& actionName) const;

    // Get an action by name (returns nullptr if not found)
    const InputAction* getAction(const std::string& actionName) const;

    // Update all actions from raw input state — call once per frame
    void update(const Input& input);

    // Digital queries
    bool isPressed(const std::string& actionName) const;
    bool isReleased(const std::string& actionName) const;
    bool isHeld(const std::string& actionName) const;

    // Axis queries
    float getAxis(const std::string& actionName) const;
    Vec2 getAxis2D(const std::string& actionName) const;

    const std::string& getName() const { return m_name; }
    size_t getActionCount() const { return m_actions.size(); }

private:
    std::string m_name = "Default";
    std::unordered_map<std::string, InputAction> m_actions;
};

} // namespace fe
