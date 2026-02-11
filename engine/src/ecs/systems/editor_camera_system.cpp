#include <filament_engine/ecs/systems/editor_camera_system.h>
#include <filament_engine/ecs/world.h>
#include <filament_engine/ecs/components.h>
#include <filament_engine/core/input.h>

#include <cmath>
#include <algorithm>

namespace fe {

void EditorCameraSystem::update(World& world, float dt) {
    auto& registry = world.getRegistry();
    auto& input = world.getInput();

    auto view = registry.view<CameraComponent, TransformComponent>();
    for (auto entity : view) {
        auto& cam = view.get<CameraComponent>(entity);
        if (!cam.isActive) continue;

        auto& transform = view.get<TransformComponent>(entity);

        // Initialize yaw/pitch from current transform on first frame
        if (!m_initialized) {
            // Extract yaw and pitch from the current rotation quaternion
            auto rotMat = filament::math::mat3f(transform.rotation);
            Vec3 forward = rotMat * Vec3{0, 0, -1};

            m_yaw = std::atan2(forward.x, -forward.z) * (180.0f / M_PI);
            m_pitch = std::asin(std::clamp(forward.y, -1.0f, 1.0f)) * (180.0f / M_PI);
            m_initialized = true;
        }

        // Adjust movement speed with scroll wheel
        float scrollY = input.getScrollDelta().y;
        if (scrollY != 0.0f) {
            movementSpeed += scrollY * scrollSpeedStep;
            movementSpeed = std::max(0.5f, movementSpeed);
        }

        // Mouse look: only when right mouse button is held
        if (input.isMouseButtonDown(MouseButton::Right)) {
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

        // Calculate current speed (with shift multiplier)
        float speed = movementSpeed;
        if (input.isKeyDown(Key::LShift) || input.isKeyDown(Key::RShift)) {
            speed *= fastMultiplier;
        }

        // WASD movement
        Vec3 movement{0, 0, 0};
        if (input.isKeyDown(Key::W)) movement += forward;
        if (input.isKeyDown(Key::S)) movement -= forward;
        if (input.isKeyDown(Key::A)) movement -= right;
        if (input.isKeyDown(Key::D)) movement += right;
        if (input.isKeyDown(Key::Q)) movement -= up;
        if (input.isKeyDown(Key::E)) movement += up;

        // Apply movement if any key is pressed
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
