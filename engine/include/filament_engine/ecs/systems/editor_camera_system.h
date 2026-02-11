#pragma once

#include <filament_engine/ecs/system.h>

namespace fe {

class Input;

// Editor-style camera controller system.
// Provides FPS-like camera movement and mouse look (right-click drag).
// Uses InputActions for all bindings — keys can be remapped via InputMap.
//
// Registered actions (created in init()):
//   "EditorMoveX"     — A/D (Axis1D: -1..+1)
//   "EditorMoveY"     — Q/E (Axis1D: -1..+1)
//   "EditorMoveZ"     — W/S (Axis1D: -1..+1, forward/backward)
//   "EditorLook"      — Right mouse button (Digital)
//   "EditorFast"      — Left Shift (Digital)
//   "EditorSpeed"     — Scroll wheel (Axis1D)
class EditorCameraSystem : public System {
public:
    EditorCameraSystem() { priority = 290; } // runs before CameraSystem (300)

    void init(World& world) override;
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
