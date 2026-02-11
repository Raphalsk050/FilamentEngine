#pragma once

#include <filament_engine/math/types.h>
#include <filament_engine/resources/resource_handle.h>

#include <entt/entt.hpp>

#include <string>

namespace fe {

// Forward declarations
struct Mesh;
class MaterialWrapper;

// Synced to Filament's TransformManager when dirty.
struct TransformComponent {
    Vec3 position{0, 0, 0};
    Quat rotation = Quat{1, 0, 0, 0}; // identity quaternion (w, x, y, z)
    Vec3 scale{1, 1, 1};
    entt::entity parent{entt::null};
    bool dirty = true; // set to true when the transform needs syncing to Filament
};
struct TagComponent {
    std::string name;
};
struct MeshRendererComponent {
    ResourceHandle<Mesh> mesh;
    ResourceHandle<MaterialWrapper> material;
    bool castShadows = true;
    bool receiveShadows = true;
    bool initialized = false; // internal: whether Filament renderable has been created
};
struct CameraComponent {
    float fov = 60.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    bool isActive = false;
    bool dirty = true;
};
struct LightComponent {
    enum class Type { Directional, Point, Spot };

    Type type = Type::Point;
    Vec3 color{1, 1, 1};
    float intensity = 100000.0f;
    float radius = 10.0f;        // for point/spot lights
    float innerConeAngle = 0.0f; // for spot lights (radians)
    float outerConeAngle = 0.5f; // for spot lights (radians)
    bool castShadows = false;
    bool initialized = false;    // internal: whether Filament light has been created
};

} // namespace fe
