#pragma once

namespace fe {

class World;

// Base class for ECS systems.
// Systems process entities with specific component combinations each frame.
class System {
public:
    virtual ~System() = default;

    // Called once when the system is registered
    virtual void init(World& world) {}

    // Called every frame
    virtual void update(World& world, float dt) = 0;

    // Called when the system is removed or the world is destroyed
    virtual void shutdown(World& world) {}

    // Execution priority: lower values run first
    int priority = 0;
};

} // namespace fe
