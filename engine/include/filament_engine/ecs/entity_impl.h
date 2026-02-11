#pragma once

// Template implementations for Entity methods.
// This file must be included after both entity.h and world.h are defined.
// It is automatically included by world.h at the bottom.

#include <filament_engine/ecs/entity.h>

namespace fe {

template <typename T, typename... Args>
T& Entity::addComponent(Args&&... args) {
    return m_world->addComponent<T>(m_handle, std::forward<Args>(args)...);
}

template <typename T>
T& Entity::getComponent() {
    return m_world->getComponent<T>(m_handle);
}

template <typename T>
const T& Entity::getComponent() const {
    return m_world->getComponent<T>(m_handle);
}

template <typename T>
T* Entity::tryGetComponent() {
    return m_world->tryGetComponent<T>(m_handle);
}

template <typename T>
bool Entity::hasComponent() const {
    return m_world->hasComponent<T>(m_handle);
}

template <typename T>
void Entity::removeComponent() {
    m_world->removeComponent<T>(m_handle);
}

} // namespace fe
