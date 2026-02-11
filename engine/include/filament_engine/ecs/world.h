#pragma once

#include <filament_engine/ecs/entity_bridge.h>
#include <filament_engine/ecs/components.h>
#include <filament_engine/ecs/system.h>
#include <filament_engine/core/input.h>

#include <entt/entt.hpp>

#include <memory>
#include <vector>
#include <algorithm>

namespace fe {

class RenderContext;

// Wraps entt::registry with entity bridge and system dispatch.
class World {
public:
    World(RenderContext& renderContext, Input& input);
    ~World();
    entt::entity createEntity(const std::string& name = "Entity");
    void destroyEntity(entt::entity entity);
    template <typename T, typename... Args>
    T& addComponent(entt::entity entity, Args&&... args) {
        return m_registry.emplace<T>(entity, std::forward<Args>(args)...);
    }

    template <typename T>
    T& getComponent(entt::entity entity) {
        return m_registry.get<T>(entity);
    }

    template <typename T>
    const T& getComponent(entt::entity entity) const {
        return m_registry.get<T>(entity);
    }

    template <typename T>
    T* tryGetComponent(entt::entity entity) {
        return m_registry.try_get<T>(entity);
    }

    template <typename T>
    bool hasComponent(entt::entity entity) const {
        return m_registry.all_of<T>(entity);
    }

    template <typename T>
    void removeComponent(entt::entity entity) {
        m_registry.remove<T>(entity);
    }
    template <typename T, typename... Args>
    T& registerSystem(Args&&... args) {
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *system;
        m_systems.push_back(std::move(system));
        // Sort by priority
        std::sort(m_systems.begin(), m_systems.end(),
            [](const auto& a, const auto& b) { return a->priority < b->priority; });
        ref.init(*this);
        return ref;
    }

    void updateSystems(float dt);
    void shutdownSystems();
    entt::registry& getRegistry() { return m_registry; }
    const entt::registry& getRegistry() const { return m_registry; }
    EntityBridge& getEntityBridge() { return m_entityBridge; }
    RenderContext& getRenderContext() { return m_renderContext; }
    Input& getInput() { return m_input; }

private:
    entt::registry m_registry;
    EntityBridge m_entityBridge;
    RenderContext& m_renderContext;
    Input& m_input;
    std::vector<std::unique_ptr<System>> m_systems;
};

} // namespace fe
