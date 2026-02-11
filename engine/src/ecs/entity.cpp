#include <filament_engine/ecs/entity.h>
#include <filament_engine/ecs/world.h>
#include <filament_engine/ecs/components.h>

namespace fe {

Entity::Entity(entt::entity handle, World* world)
    : m_handle(handle), m_world(world) {
}

bool Entity::isValid() const {
    return m_world != nullptr && m_handle != entt::null &&
           m_world->getRegistry().valid(m_handle);
}

void Entity::destroy() {
    if (isValid()) {
        m_world->destroyEntity(m_handle);
        m_handle = entt::null;
        m_world = nullptr;
    }
}

TransformComponent& Entity::transform() {
    return m_world->getComponent<TransformComponent>(m_handle);
}

const TransformComponent& Entity::transform() const {
    return m_world->getComponent<TransformComponent>(m_handle);
}

const std::string& Entity::name() const {
    return m_world->getComponent<TagComponent>(m_handle).name;
}

// Template method implementations — must be in a header that includes world.h,
// so we define them here where world.h is already included.
// However, since they need to be accessible from user code, we'll define them
// in entity_impl.h (included by world.h or filament_engine.h).

} // namespace fe
