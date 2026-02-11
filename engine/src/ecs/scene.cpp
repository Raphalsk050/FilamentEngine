#include <filament_engine/ecs/scene.h>
#include <filament_engine/ecs/world.h>
#include <filament_engine/ecs/entity.h>
#include <filament_engine/core/log.h>

#include <algorithm>

namespace fe {

Scene::Scene(World& world, const std::string& name)
    : m_world(world), m_name(name) {
    FE_LOG_DEBUG("Scene '%s' created", m_name.c_str());
}

Scene::~Scene() {
    destroyAll();
}

Entity Scene::createEntity(const std::string& name) {
    Entity entity = m_world.createEntity(name);
    m_entities.push_back(entity.getHandle());
    return entity;
}

void Scene::destroyEntity(Entity entity) {
    auto it = std::find(m_entities.begin(), m_entities.end(), entity.getHandle());
    if (it != m_entities.end()) {
        m_world.destroyEntity(entity.getHandle());
        m_entities.erase(it);
    }
}

void Scene::destroyAll() {
    // Destroy in reverse order for safety
    for (auto it = m_entities.rbegin(); it != m_entities.rend(); ++it) {
        if (m_world.getRegistry().valid(*it)) {
            m_world.destroyEntity(*it);
        }
    }
    m_entities.clear();
    FE_LOG_DEBUG("Scene '%s' cleared", m_name.c_str());
}

bool Scene::contains(entt::entity entity) const {
    return std::find(m_entities.begin(), m_entities.end(), entity) != m_entities.end();
}

} // namespace fe
