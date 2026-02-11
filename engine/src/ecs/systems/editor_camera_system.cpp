#include <filament_engine/ecs/systems/editor_camera_system.h>
#include <filament_engine/ecs/world.h>
#include <filament_engine/ecs/components.h>
#include <filament_engine/core/input.h>
#include <filament_engine/core/input_map.h>

#include <cmath>
#include <algorithm>

namespace fe {

void EditorCameraSystem::init(World& world) {
    auto& map = world.getInputMap();

    // Horizontal strafe: A(-1) / D(+1)
    auto& moveX = map.createAction("EditorMoveX", InputActionType::Axis1D);
    moveX.addBinding({InputSource::Key, Key::D, {}, 1.0f});
    moveX.addBinding({InputSource::Key, Key::A, {}, -1.0f});

    // Vertical movement: E(+1) / Q(-1)
    auto& moveY = map.createAction("EditorMoveY", InputActionType::Axis1D);
    moveY.addBinding({InputSource::Key, Key::E, {}, 1.0f});
    moveY.addBinding({InputSource::Key, Key::Q, {}, -1.0f});

    // Forward/backward: W(+1) / S(-1)
    auto& moveZ = map.createAction("EditorMoveZ", InputActionType::Axis1D);
    moveZ.addBinding({InputSource::Key, Key::W, {}, 1.0f});
    moveZ.addBinding({InputSource::Key, Key::S, {}, -1.0f});

    // Mouse look toggle: right mouse button
    auto& look = map.createAction("EditorLook", InputActionType::Digital);
    look.addBinding({InputSource::MouseButton, Key::Unknown, MouseButton::Right});

    // Sprint modifier: left/right shift
    auto& fast = map.createAction("EditorFast", InputActionType::Digital);
    fast.addBinding({InputSource::Key, Key::LShift});
    fast.addBinding({InputSource::Key, Key::RShift});

    // Speed adjustment: scroll wheel Y
    auto& speed = map.createAction("EditorSpeed", InputActionType::Axis1D);
    speed.addBinding({InputSource::ScrollY, Key::Unknown, {}, 1.0f});
}

void EditorCameraSystem::update(World& world, float dt) {
    auto& registry = world.getRegistry();
    auto& input = world.getInput();
    auto& map = world.getInputMap();

    auto view = registry.view<CameraComponent, TransformComponent>();
    for (auto entity : view) {
        auto& cam = view.get<CameraComponent>(entity);
        if (!cam.isActive) continue;

        auto& transform = view.get<TransformComponent>(entity);

        // Initialize yaw/pitch from current transform on first frame
        if (!m_initialized) {
            auto rotMat = filament::math::mat3f(transform.rotation);
            Vec3 forward = rotMat * Vec3{0, 0, -1};

            m_yaw = std::atan2(forward.x, -forward.z) * (180.0f / M_PI);
            m_pitch = std::asin(std::clamp(forward.y, -1.0f, 1.0f)) * (180.0f / M_PI);
            m_initialized = true;
        }

        // Adjust movement speed with scroll wheel
        float scrollValue = map.getAxis("EditorSpeed");
        if (scrollValue != 0.0f) {
            movementSpeed += scrollValue * scrollSpeedStep;
            movementSpeed = std::max(0.5f, movementSpeed);
        }

        // Mouse look: only when EditorLook action is held (right mouse button)
        if (map.isHeld("EditorLook")) {
            Vec2 mouseDelta = input.getMouseDelta();
            m_yaw += mouseDelta.x * mouseSensitivity;
            m_pitch -= mouseDelta.y * mouseSensitivity;

            // Clamp pitch to avoid gimbal lock
            m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
        }

        // Build rotation quaternion from yaw and pitch (Euler angles)
        float yawRad = m_yaw * (M_PI / 180.0f);
        float pitchRad = m_pitch * (M_PI / 180.0f);

        // Quaternion from Euler: yaw around Y axis, then pitch around X axis
        Quat yawQuat{std::cos(yawRad / 2.0f), 0, std::sin(yawRad / 2.0f), 0};
        Quat pitchQuat{std::cos(pitchRad / 2.0f), std::sin(pitchRad / 2.0f), 0, 0};
        transform.rotation = yawQuat * pitchQuat;

        // Compute local direction vectors from the rotation
        auto rotMat = filament::math::mat3f(transform.rotation);
        Vec3 forward = rotMat * Vec3{0, 0, -1};
        Vec3 right = rotMat * Vec3{1, 0, 0};
        Vec3 up = Vec3{0, 1, 0}; // world up for consistent movement

        // Calculate current speed (with sprint modifier)
        float speed = movementSpeed;
        if (map.isHeld("EditorFast")) {
            speed *= fastMultiplier;
        }

        // Movement driven by InputActions instead of raw keys
        float moveX = map.getAxis("EditorMoveX");
        float moveY = map.getAxis("EditorMoveY");
        float moveZ = map.getAxis("EditorMoveZ");

        Vec3 movement{0, 0, 0};
        movement += forward * moveZ;  // W/S
        movement += right * moveX;    // A/D
        movement += up * moveY;       // Q/E

        // Apply movement if any axis is active
        float lengthSq = movement.x * movement.x + movement.y * movement.y + movement.z * movement.z;
        if (lengthSq > 0.0f) {
            float length = std::sqrt(lengthSq);
            movement = movement / length; // normalize
            transform.position += movement * speed * dt;
        }

        transform.dirty = true;
        break; // only process the first active camera
    }
}

} // namespace fe
