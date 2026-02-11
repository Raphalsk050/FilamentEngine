#pragma once

#include <filament_engine/ecs/system.h>

namespace fe {

// Syncs TransformComponent data from EnTT to Filament's TransformManager.
// Uses batch transactions for performance when many transforms change.
class TransformSyncSystem : public System {
public:
    TransformSyncSystem() { priority = 100; } // runs before rendering systems

    void update(World& world, float dt) override;
};

} // namespace fe
