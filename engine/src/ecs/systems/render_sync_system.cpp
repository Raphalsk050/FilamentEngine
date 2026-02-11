#include <filament_engine/ecs/systems/render_sync_system.h>
#include <filament_engine/ecs/world.h>
#include <filament_engine/ecs/components.h>
#include <filament_engine/rendering/render_context.h>
#include <filament_engine/resources/resource_manager.h>

#include <filament/RenderableManager.h>
#include <filament/Scene.h>

namespace fe {

void RenderSyncSystem::update(World& world, float dt) {
    auto& registry = world.getRegistry();
    auto& bridge = world.getEntityBridge();
    auto& renderCtx = world.getRenderContext();
    auto* engine = renderCtx.getEngine();
    auto* scene = renderCtx.getScene();

    auto view = registry.view<MeshRendererComponent, FilamentEntityComponent>();
    for (auto entity : view) {
        auto& meshRenderer = view.get<MeshRendererComponent>(entity);
        if (meshRenderer.initialized) continue;
        if (!meshRenderer.mesh.isValid() || !meshRenderer.material.isValid()) continue;

        auto& fec = view.get<FilamentEntityComponent>(entity);
        auto filamentEntity = fec.filamentEntity;

        // Get the actual mesh and material from ResourceManager
        auto* resourceMgr = ResourceManager::getInstance();
        if (!resourceMgr) continue;

        auto* mesh = resourceMgr->getMesh(meshRenderer.mesh);
        auto* material = resourceMgr->getMaterial(meshRenderer.material);
        if (!mesh || !material) continue;

        // Build the Filament renderable
        filament::RenderableManager::Builder(1)
            .boundingBox(mesh->boundingBox)
            .material(0, material->getInstance())
            .geometry(0, filament::RenderableManager::PrimitiveType::TRIANGLES,
                      mesh->vertexBuffer, mesh->indexBuffer,
                      0, mesh->indexCount)
            .culling(false)
            .receiveShadows(meshRenderer.receiveShadows)
            .castShadows(meshRenderer.castShadows)
            .build(*engine, filamentEntity);

        // Add to scene
        scene->addEntity(filamentEntity);

        meshRenderer.initialized = true;
    }
}

} // namespace fe
