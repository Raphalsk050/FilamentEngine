#pragma once

#include <filament_engine/ecs/system.h>

namespace fe {

class Input;

// Editor-style camera controller system.
// Provides FPS-like camera movement with WASD keys and mouse look (right-click drag).
// Scroll wheel adjusts movement speed. Shift for fast movement.
class EditorCameraSystem : public System {
public:
    EditorCameraSystem() { priority = 290; } // runs before CameraSystem (300)

    void update(World& world, float dt) override;

    // Configuration
    float movementSpeed = 5.0f;
    float mouseSensitivity = 0.15f;
    float fastMultiplier = 3.0f;
    float scrollSpeedStep = 1.0f;

private:
    float m_yaw = 0.0f;     // horizontal rotation in degrees
    float m_pitch = 0.0f;   // vertical rotation in degrees
    bool m_initialized = false;
};

} // namespace fe
