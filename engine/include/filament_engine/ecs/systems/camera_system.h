#pragma once

#include <filament_engine/ecs/system.h>

namespace fe {

// Syncs the active CameraComponent to Filament's Camera.
// Updates projection and position/orientation from TransformComponent.
class CameraSystem : public System {
public:
    CameraSystem() { priority = 300; } // runs after transform and render sync

    void update(World& world, float dt) override;
};

} // namespace fe
