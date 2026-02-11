#include <filament_engine/core/input_action.h>

namespace fe {

InputAction::InputAction(std::string name, InputActionType type)
    : m_name(std::move(name)), m_type(type) {
}

void InputAction::addBinding(InputBinding binding) {
    m_bindings.push_back(binding);
}

void InputAction::clearBindings() {
    m_bindings.clear();
}

void InputAction::beginFrame() {
    // Reset per-frame flags — they will be set by evaluate()
    m_state.pressed = false;
    m_state.released = false;
}

void InputAction::evaluate(const Input& input) {
    m_wasHeld = m_state.held;

    switch (m_type) {
        case InputActionType::Digital: {
            bool active = false;
            for (const auto& binding : m_bindings) {
                switch (binding.source) {
                    case InputSource::Key:
                        if (input.isKeyDown(binding.key)) active = true;
                        break;
                    case InputSource::MouseButton:
                        if (input.isMouseButtonDown(binding.mouseButton)) active = true;
                        break;
                    default:
                        break;
                }
                if (active) break;
            }
            m_state.held = active;
            m_state.value = active ? 1.0f : 0.0f;
            m_state.pressed = active && !m_wasHeld;
            m_state.released = !active && m_wasHeld;
            break;
        }
        case InputActionType::Axis1D: {
            float total = 0.0f;
            for (const auto& binding : m_bindings) {
                switch (binding.source) {
                    case InputSource::Key:
                        if (input.isKeyDown(binding.key)) {
                            total += binding.scale;
                        }
                        break;
                    case InputSource::MouseButton:
                        if (input.isMouseButtonDown(binding.mouseButton)) {
                            total += binding.scale;
                        }
                        break;
                    case InputSource::MouseAxisX:
                        total += input.getMouseDelta().x * binding.scale;
                        break;
                    case InputSource::MouseAxisY:
                        total += input.getMouseDelta().y * binding.scale;
                        break;
                    case InputSource::ScrollX:
                        total += input.getScrollDelta().x * binding.scale;
                        break;
                    case InputSource::ScrollY:
                        total += input.getScrollDelta().y * binding.scale;
                        break;
                }
            }
            m_state.value = total;
            m_state.held = (total != 0.0f);
            m_state.pressed = m_state.held && !m_wasHeld;
            m_state.released = !m_state.held && m_wasHeld;
            break;
        }
        case InputActionType::Axis2D: {
            Vec2 total{0.0f, 0.0f};
            for (const auto& binding : m_bindings) {
                float contribution = 0.0f;
                switch (binding.source) {
                    case InputSource::Key:
                        if (input.isKeyDown(binding.key)) {
                            contribution = binding.scale;
                        }
                        break;
                    case InputSource::MouseButton:
                        if (input.isMouseButtonDown(binding.mouseButton)) {
                            contribution = binding.scale;
                        }
                        break;
                    case InputSource::MouseAxisX:
                        contribution = input.getMouseDelta().x * binding.scale;
                        break;
                    case InputSource::MouseAxisY:
                        contribution = input.getMouseDelta().y * binding.scale;
                        break;
                    case InputSource::ScrollX:
                        contribution = input.getScrollDelta().x * binding.scale;
                        break;
                    case InputSource::ScrollY:
                        contribution = input.getScrollDelta().y * binding.scale;
                        break;
                }
                if (binding.axisIndex == 0) {
                    total.x += contribution;
                } else {
                    total.y += contribution;
                }
            }
            m_state.axis2D = total;
            m_state.value = 0.0f; // not meaningful for 2D
            m_state.held = (total.x != 0.0f || total.y != 0.0f);
            m_state.pressed = m_state.held && !m_wasHeld;
            m_state.released = !m_state.held && m_wasHeld;
            break;
        }
    }
}

} // namespace fe
