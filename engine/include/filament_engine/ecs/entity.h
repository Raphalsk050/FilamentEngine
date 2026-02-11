#pragma once

#include <filament_engine/math/types.h>

#include <entt/entt.hpp>

#include <string>

namespace fe {

class World;
struct TransformComponent;
struct TagComponent;

// Lightweight entity handle — wraps entt::entity + World* for an ergonomic API.
// Modeled after Unity's GameObject and Unreal's AActor.
//
// Usage:
//   auto entity = world.createEntity("Player");
//   entity.addComponent<MeshRendererComponent>();
//   entity.transform().position = {0, 1, 0};
//   entity.getComponent<CameraComponent>().fov = 90.0f;
class Entity {
public:
    Entity() = default;
    Entity(entt::entity handle, World* world);

    // Implicit conversion to entt::entity for backwards compatibility
    operator entt::entity() const { return m_handle; }

    // Component management — same API as Unity's GetComponent/AddComponent
    template <typename T, typename... Args>
    T& addComponent(Args&&... args);

    template <typename T>
    T& getComponent();

    template <typename T>
    const T& getComponent() const;

    template <typename T>
    T* tryGetComponent();

    template <typename T>
    bool hasComponent() const;

    template <typename T>
    void removeComponent();

    // Convenience shortcuts
    TransformComponent& transform();
    const TransformComponent& transform() const;
    const std::string& name() const;

    // Lifecycle
    void destroy();
    bool isValid() const;

    // Identity
    uint32_t getId() const { return static_cast<uint32_t>(m_handle); }
    entt::entity getHandle() const { return m_handle; }
    World* getWorld() const { return m_world; }

    bool operator==(const Entity& other) const { return m_handle == other.m_handle; }
    bool operator!=(const Entity& other) const { return m_handle != other.m_handle; }

private:
    entt::entity m_handle{entt::null};
    World* m_world = nullptr;
};

} // namespace fe

// Hash support for containers
namespace std {
template <>
struct hash<fe::Entity> {
    size_t operator()(const fe::Entity& entity) const {
        return std::hash<uint32_t>{}(static_cast<uint32_t>(entity.getHandle()));
    }
};
} // namespace std
