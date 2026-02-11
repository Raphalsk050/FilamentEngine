#include <filament_engine/core/input_map.h>
#include <filament_engine/core/log.h>

namespace fe {

InputMap::InputMap(std::string name)
    : m_name(std::move(name)) {
}

InputAction& InputMap::createAction(const std::string& actionName, InputActionType type) {
    auto [it, inserted] = m_actions.try_emplace(actionName, actionName, type);
    if (!inserted) {
        FE_LOG_WARN("InputMap '%s': action '%s' already exists, returning existing",
            m_name.c_str(), actionName.c_str());
    }
    return it->second;
}

void InputMap::addBinding(const std::string& actionName, InputBinding binding) {
    auto it = m_actions.find(actionName);
    if (it == m_actions.end()) {
        FE_LOG_WARN("InputMap '%s': action '%s' not found, cannot add binding",
            m_name.c_str(), actionName.c_str());
        return;
    }
    it->second.addBinding(binding);
}

void InputMap::removeAction(const std::string& actionName) {
    m_actions.erase(actionName);
}

bool InputMap::hasAction(const std::string& actionName) const {
    return m_actions.find(actionName) != m_actions.end();
}

const InputAction* InputMap::getAction(const std::string& actionName) const {
    auto it = m_actions.find(actionName);
    return (it != m_actions.end()) ? &it->second : nullptr;
}

void InputMap::update(const Input& input) {
    for (auto& [name, action] : m_actions) {
        action.beginFrame();
        action.evaluate(input);
    }
}

bool InputMap::isPressed(const std::string& actionName) const {
    auto it = m_actions.find(actionName);
    return (it != m_actions.end()) ? it->second.getState().pressed : false;
}

bool InputMap::isReleased(const std::string& actionName) const {
    auto it = m_actions.find(actionName);
    return (it != m_actions.end()) ? it->second.getState().released : false;
}

bool InputMap::isHeld(const std::string& actionName) const {
    auto it = m_actions.find(actionName);
    return (it != m_actions.end()) ? it->second.getState().held : false;
}

float InputMap::getAxis(const std::string& actionName) const {
    auto it = m_actions.find(actionName);
    return (it != m_actions.end()) ? it->second.getState().value : 0.0f;
}

Vec2 InputMap::getAxis2D(const std::string& actionName) const {
    auto it = m_actions.find(actionName);
    return (it != m_actions.end()) ? it->second.getState().axis2D : Vec2{0.0f, 0.0f};
}

} // namespace fe
