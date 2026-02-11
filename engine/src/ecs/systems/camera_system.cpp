#include <filament_engine/ecs/systems/camera_system.h>
#include <filament_engine/ecs/world.h>
#include <filament_engine/ecs/components.h>
#include <filament_engine/rendering/render_context.h>

#include <filament/Camera.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include <math/mat4.h>
#include <math/vec3.h>

namespace fe {

void CameraSystem::update(World& world, float dt) {
    auto& registry = world.getRegistry();
    auto& renderCtx = world.getRenderContext();

    auto view = registry.view<CameraComponent, TransformComponent>();
    for (auto entity : view) {
        auto& cam = view.get<CameraComponent>(entity);
        if (!cam.isActive) continue;

        auto& transform = view.get<TransformComponent>(entity);
        auto* camera = renderCtx.getActiveCamera();
        if (!camera) continue;

        // Update projection if camera params changed
        if (cam.dirty) {
            auto viewport = renderCtx.getView()->getViewport();
            float aspect = static_cast<float>(viewport.width) / static_cast<float>(viewport.height);
            camera->setProjection(cam.fov, aspect, cam.nearPlane, cam.farPlane);
            cam.dirty = false;
        }

        // Compute forward and up vectors from the rotation quaternion
        // Filament uses a right-handed coordinate system with -Z forward
        auto rotationMat = filament::math::mat3f(transform.rotation);
        Vec3 forward = rotationMat * Vec3{0, 0, -1};
        Vec3 up = rotationMat * Vec3{0, 1, 0};

        Vec3 target = transform.position + forward;
        camera->lookAt(transform.position, target, up);
    }
}

} // namespace fe
