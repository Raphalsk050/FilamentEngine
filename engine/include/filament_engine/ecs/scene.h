#pragma once

#include <entt/entt.hpp>

#include <string>
#include <vector>

namespace fe {

class World;
class Entity;

// Scene — a logical group of entities for batch operations (load/unload).
// Each scene tracks which entities belong to it. Destroying a scene
// destroys all its entities.
class Scene {
public:
    Scene(World& world, const std::string& name);
    ~Scene();

    // Create an entity that belongs to this scene
    Entity createEntity(const std::string& name = "Entity");

    // Destroy a specific entity from this scene
    void destroyEntity(Entity entity);

    // Destroy all entities in this scene
    void destroyAll();

    // Query
    const std::string& getName() const { return m_name; }
    const std::vector<entt::entity>& getEntities() const { return m_entities; }
    size_t getEntityCount() const { return m_entities.size(); }
    bool contains(entt::entity entity) const;

private:
    World& m_world;
    std::string m_name;
    std::vector<entt::entity> m_entities;
};

} // namespace fe
