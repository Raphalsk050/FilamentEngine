#include <filament_engine/ecs/entity_bridge.h>

namespace fe {

utils::Entity EntityBridge::link(entt::registry& registry, entt::entity entity) {
    // Create a new Filament entity
    auto filamentEntity = utils::EntityManager::get().create();

    // Store the Filament entity as a component on the EnTT entity
    registry.emplace<FilamentEntityComponent>(entity, filamentEntity);

    // Register reverse lookup
    m_filamentToEntt[filamentEntity.getId()] = entity;

    return filamentEntity;
}

void EntityBridge::unlink(entt::registry& registry, entt::entity entity) {
    auto* comp = registry.try_get<FilamentEntityComponent>(entity);
    if (!comp) return;

    // Remove reverse lookup
    m_filamentToEntt.erase(comp->filamentEntity.getId());

    // Destroy the Filament entity
    utils::EntityManager::get().destroy(comp->filamentEntity);

    // Remove the component
    registry.remove<FilamentEntityComponent>(entity);
}

entt::entity EntityBridge::getEnttEntity(utils::Entity filamentEntity) const {
    auto it = m_filamentToEntt.find(filamentEntity.getId());
    if (it != m_filamentToEntt.end()) {
        return it->second;
    }
    return entt::null;
}

utils::Entity EntityBridge::getFilamentEntity(const entt::registry& registry, entt::entity entity) const {
    auto* comp = registry.try_get<FilamentEntityComponent>(entity);
    if (comp) {
        return comp->filamentEntity;
    }
    return utils::Entity{};
}

bool EntityBridge::hasFilamentEntity(const entt::registry& registry, entt::entity entity) const {
    return registry.all_of<FilamentEntityComponent>(entity);
}

} // namespace fe
