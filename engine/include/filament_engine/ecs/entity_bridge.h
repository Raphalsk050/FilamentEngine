#pragma once

#include <utils/Entity.h>
#include <utils/EntityManager.h>

#include <entt/entt.hpp>

#include <unordered_map>

namespace fe {

// Component stored on EnTT entities that have a Filament counterpart
struct FilamentEntityComponent {
    utils::Entity filamentEntity;
};

// Bidirectional mapping between EnTT entities and Filament entities.
// Each game entity that needs rendering gets both an EnTT entity and a Filament entity.
class EntityBridge {
public:
    // Creates a new Filament entity and links it to the given EnTT entity
    utils::Entity link(entt::registry& registry, entt::entity entity);

    // Destroys the Filament entity and removes the mapping
    void unlink(entt::registry& registry, entt::entity entity);

    // Lookup: Filament → EnTT
    entt::entity getEnttEntity(utils::Entity filamentEntity) const;

    // Lookup: EnTT → Filament (via component)
    utils::Entity getFilamentEntity(const entt::registry& registry, entt::entity entity) const;

    // Check if an EnTT entity has a Filament counterpart
    bool hasFilamentEntity(const entt::registry& registry, entt::entity entity) const;

private:
    std::unordered_map<uint32_t, entt::entity> m_filamentToEntt;
};

} // namespace fe
