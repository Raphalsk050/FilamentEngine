#include <filament_engine/ecs/world.h>
#include <filament_engine/rendering/render_context.h>
#include <filament_engine/core/log.h>

#include <filament/TransformManager.h>
#include <filament/RenderableManager.h>
#include <filament/LightManager.h>

namespace fe {

World::World(RenderContext& renderContext, Input& input)
    : m_renderContext(renderContext), m_input(input) {
    FE_LOG_INFO("World created");
}

World::~World() {
    shutdownSystems();

    // Destroy all Filament components from entities before ResourceManager cleans up
    // materials. This prevents "MaterialInstance still in use by Renderable" errors.
    auto* engine = m_renderContext.getEngine();
    auto* scene = m_renderContext.getScene();
    auto view = m_registry.view<FilamentEntityComponent>();
    for (auto entity : view) {
        auto& fec = view.get<FilamentEntityComponent>(entity);
        auto filamentEntity = fec.filamentEntity;

        // Remove from scene
        scene->remove(filamentEntity);

        // Destroy Filament components (renderable, light, transform)
        auto& rcm = engine->getRenderableManager();
        if (rcm.hasComponent(filamentEntity)) {
            engine->destroy(filamentEntity);
        }

        auto& lcm = engine->getLightManager();
        if (lcm.hasComponent(filamentEntity)) {
            engine->destroy(filamentEntity);
        }

        auto& tcm = engine->getTransformManager();
        if (tcm.hasComponent(filamentEntity)) {
            tcm.destroy(filamentEntity);
        }

        utils::EntityManager::get().destroy(filamentEntity);
    }

    m_registry.clear();

    FE_LOG_INFO("World destroyed");
}

entt::entity World::createEntity(const std::string& name) {
    auto entity = m_registry.create();

    // Every entity gets a tag and transform by default
    m_registry.emplace<TagComponent>(entity, name);
    m_registry.emplace<TransformComponent>(entity);

    // Create the Filament entity and link it
    m_entityBridge.link(m_registry, entity);

    // Create a Filament TransformManager component for this entity
    auto filamentEntity = m_entityBridge.getFilamentEntity(m_registry, entity);
    auto& tcm = m_renderContext.getTransformManager();
    tcm.create(filamentEntity);

    return entity;
}

void World::destroyEntity(entt::entity entity) {
    if (!m_registry.valid(entity)) return;

    // Clean up Filament-side resources
    auto filamentEntity = m_entityBridge.getFilamentEntity(m_registry, entity);
    if (!filamentEntity.isNull()) {
        auto* engine = m_renderContext.getEngine();

        // Remove from scene
        m_renderContext.getScene()->remove(filamentEntity);

        // Destroy Filament components
        auto& tcm = m_renderContext.getTransformManager();
        if (tcm.hasComponent(filamentEntity)) {
            tcm.destroy(filamentEntity);
        }

        auto& rcm = m_renderContext.getRenderableManager();
        if (rcm.hasComponent(filamentEntity)) {
            engine->destroy(filamentEntity);
        }

        auto& lcm = m_renderContext.getLightManager();
        if (lcm.hasComponent(filamentEntity)) {
            engine->destroy(filamentEntity);
        }
    }

    // Unlink from bridge
    m_entityBridge.unlink(m_registry, entity);

    // Destroy EnTT entity
    m_registry.destroy(entity);
}

void World::updateSystems(float dt) {
    for (auto& system : m_systems) {
        system->update(*this, dt);
    }
}

void World::shutdownSystems() {
    for (auto& system : m_systems) {
        system->shutdown(*this);
    }
    m_systems.clear();
}

} // namespace fe
