#pragma once

#include <filament_engine/ecs/system.h>

namespace fe {

// Syncs LightComponent to Filament's LightManager.
// Creates Filament lights when components are first seen, and updates them.
class LightSystem : public System {
public:
    LightSystem() { priority = 250; } // runs after render sync, before camera

    void update(World& world, float dt) override;
};

} // namespace fe
