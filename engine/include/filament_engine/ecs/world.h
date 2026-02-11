#pragma once

#include <filament_engine/ecs/entity.h>
#include <filament_engine/ecs/entity_bridge.h>
#include <filament_engine/ecs/components.h>
#include <filament_engine/ecs/system.h>
#include <filament_engine/core/input.h>
#include <filament_engine/core/input_map.h>

#include <entt/entt.hpp>

#include <memory>
#include <vector>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <functional>

namespace fe {

class RenderContext;
class Scene;

// Wraps entt::registry with entity bridge, system dispatch, and scene management.
// Provides a Unity/Unreal-like API for entity and component manipulation.
class World {
public:
    World(RenderContext& renderContext, Input& input, InputMap& inputMap);
    ~World();

    // Entity management — returns Entity handle for ergonomic API
    Entity createEntity(const std::string& name = "Entity");
    void destroyEntity(entt::entity entity);
    void destroyEntity(Entity entity);

    // Component management (also available via Entity handle)
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

    // System management
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

    // Ergonomic iteration — callback receives (Entity, Components&...)
    template <typename... Components, typename Func>
    void forEach(Func&& func) {
        auto view = m_registry.view<Components...>();
        for (auto entity : view) {
            func(Entity(entity, this), view.template get<Components>(entity)...);
        }
    }

    // Scene management
    Scene& createScene(const std::string& name);
    Scene* getScene(const std::string& name);
    void destroyScene(const std::string& name);

    // Accessors
    entt::registry& getRegistry() { return m_registry; }
    const entt::registry& getRegistry() const { return m_registry; }
    EntityBridge& getEntityBridge() { return m_entityBridge; }
    RenderContext& getRenderContext() { return m_renderContext; }
    Input& getInput() { return m_input; }
    InputMap& getInputMap() { return m_inputMap; }

private:
    entt::registry m_registry;
    EntityBridge m_entityBridge;
    RenderContext& m_renderContext;
    Input& m_input;
    InputMap& m_inputMap;
    std::vector<std::unique_ptr<System>> m_systems;
    std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
};

} // namespace fe

// Include Entity template implementations now that World is fully defined
#include <filament_engine/ecs/entity_impl.h>
