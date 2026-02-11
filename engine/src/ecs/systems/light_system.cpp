#include <filament_engine/ecs/systems/light_system.h>
#include <filament_engine/ecs/world.h>
#include <filament_engine/ecs/components.h>
#include <filament_engine/rendering/render_context.h>

#include <filament/LightManager.h>
#include <filament/Scene.h>

namespace fe {

static filament::LightManager::Type toFilamentLightType(LightComponent::Type type) {
    switch (type) {
        case LightComponent::Type::Directional: return filament::LightManager::Type::DIRECTIONAL;
        case LightComponent::Type::Point:       return filament::LightManager::Type::POINT;
        case LightComponent::Type::Spot:        return filament::LightManager::Type::SPOT;
    }
    return filament::LightManager::Type::POINT;
}

void LightSystem::update(World& world, float dt) {
    auto& registry = world.getRegistry();
    auto& bridge = world.getEntityBridge();
    auto& renderCtx = world.getRenderContext();
    auto* engine = renderCtx.getEngine();
    auto* scene = renderCtx.getScene();
    auto& lightMgr = renderCtx.getLightManager();

    auto view = registry.view<LightComponent, TransformComponent, FilamentEntityComponent>();
    for (auto entity : view) {
        auto& light = view.get<LightComponent>(entity);
        auto& transform = view.get<TransformComponent>(entity);
        auto& fec = view.get<FilamentEntityComponent>(entity);
        auto filamentEntity = fec.filamentEntity;

        if (!light.initialized) {
            // Create the Filament light on first encounter
            auto builder = filament::LightManager::Builder(toFilamentLightType(light.type))
                .color({light.color.x, light.color.y, light.color.z})
                .intensity(light.intensity)
                .castShadows(light.castShadows);

            if (light.type == LightComponent::Type::Directional) {
                auto rotationMat = filament::math::mat3f(transform.rotation);
                Vec3 direction = rotationMat * Vec3{0, 0, -1};
                builder.direction(direction);
            }

            if (light.type == LightComponent::Type::Point ||
                light.type == LightComponent::Type::Spot) {
                builder.falloff(light.radius);
            }

            if (light.type == LightComponent::Type::Spot) {
                builder.spotLightCone(light.innerConeAngle, light.outerConeAngle);
            }

            builder.build(*engine, filamentEntity);

            // Add to scene
            scene->addEntity(filamentEntity);

            light.initialized = true;
        } else {
            // Continuously update light properties every frame (after first initialization)
            auto instance = lightMgr.getInstance(filamentEntity);
            if (!instance.isValid()) continue;

            // Update direction from transform rotation (for directional and spot lights)
            if (light.type == LightComponent::Type::Directional ||
                light.type == LightComponent::Type::Spot) {
                auto rotationMat = filament::math::mat3f(transform.rotation);
                Vec3 direction = rotationMat * Vec3{0, 0, -1};
                lightMgr.setDirection(instance, direction);
            }

            // Update position for point/spot lights
            if (light.type == LightComponent::Type::Point ||
                light.type == LightComponent::Type::Spot) {
                lightMgr.setPosition(instance, transform.position);
            }

            // Update color and intensity
            lightMgr.setColor(instance, {light.color.x, light.color.y, light.color.z});
            lightMgr.setIntensity(instance, light.intensity);
        }
    }
}

} // namespace fe
