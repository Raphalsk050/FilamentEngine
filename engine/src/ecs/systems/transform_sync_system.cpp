#include <filament_engine/ecs/systems/transform_sync_system.h>
#include <filament_engine/ecs/world.h>
#include <filament_engine/ecs/components.h>
#include <filament_engine/rendering/render_context.h>

#include <filament/TransformManager.h>

#include <math/mat4.h>
#include <math/vec3.h>
#include <math/quat.h>

namespace fe {

// Builds a model matrix from position, rotation, and scale
static Mat4 composeTransform(const Vec3& position, const Quat& rotation, const Vec3& scale) {
    // Scale matrix
    Mat4 s = Mat4::scaling(scale);

    // Rotation matrix from quaternion
    Mat4 r = Mat4(filament::math::mat3f(rotation));

    // Translation matrix
    Mat4 t = Mat4::translation(position);

    return t * r * s;
}

void TransformSyncSystem::update(World& world, float dt) {
    auto& registry = world.getRegistry();
    auto& bridge = world.getEntityBridge();
    auto& tcm = world.getRenderContext().getTransformManager();

    // Open a transaction for efficient batch updates
    tcm.openLocalTransformTransaction();

    auto view = registry.view<TransformComponent, FilamentEntityComponent>();
    for (auto entity : view) {
        auto& transform = view.get<TransformComponent>(entity);
        if (!transform.dirty) continue;

        auto& fec = view.get<FilamentEntityComponent>(entity);
        auto instance = tcm.getInstance(fec.filamentEntity);
        if (!instance.isValid()) continue;

        // Compose the local transform matrix
        Mat4 localTransform = composeTransform(
            transform.position, transform.rotation, transform.scale);

        tcm.setTransform(instance, localTransform);

        // Handle parent-child relationships
        if (transform.parent != entt::null && registry.valid(transform.parent)) {
            auto parentFilament = bridge.getFilamentEntity(registry, transform.parent);
            if (!parentFilament.isNull()) {
                auto parentInstance = tcm.getInstance(parentFilament);
                if (parentInstance.isValid()) {
                    tcm.setParent(instance, parentInstance);
                }
            }
        }

        transform.dirty = false;
    }

    // Commit the transaction: world transforms are now valid
    tcm.commitLocalTransformTransaction();
}

} // namespace fe
