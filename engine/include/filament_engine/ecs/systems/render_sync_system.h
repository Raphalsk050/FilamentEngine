#pragma once

#include <filament_engine/ecs/system.h>

namespace fe {

// Syncs MeshRendererComponent to Filament's RenderableManager.
// Creates Filament renderables when components are added, removes them when destroyed.
class RenderSyncSystem : public System {
public:
    RenderSyncSystem() { priority = 200; } // runs after transform sync

    void update(World& world, float dt) override;
};

} // namespace fe
