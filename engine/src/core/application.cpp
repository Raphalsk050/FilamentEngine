#include <filament_engine/core/application.h>
#include <filament_engine/core/log.h>
#include <filament_engine/ecs/world.h>
#include <filament_engine/ecs/systems/transform_sync_system.h>
#include <filament_engine/ecs/systems/render_sync_system.h>
#include <filament_engine/ecs/systems/camera_system.h>
#include <filament_engine/ecs/systems/light_system.h>
#include <filament_engine/ecs/systems/editor_camera_system.h>
#include <filament_engine/resources/resource_manager.h>

namespace fe {

Application::Application(const ApplicationConfig& config)
    : m_config(config) {
}

Application::~Application() = default;

void Application::run() {
    FE_LOG_INFO("Filament Engine v%d.%d.%d starting",
        FILAMENT_ENGINE_VERSION_MAJOR,
        FILAMENT_ENGINE_VERSION_MINOR,
        FILAMENT_ENGINE_VERSION_PATCH);

    // Create window
    m_window = std::make_unique<Window>(m_config.window);

    // Create render context
    m_renderContext = std::make_unique<RenderContext>(*m_window, m_config.backend);

    // Create resource manager
    auto resourceManager = std::make_unique<ResourceManager>(*m_renderContext->getEngine());

    // Create ECS world
    m_world = std::make_unique<World>(*m_renderContext, m_input);

    // Register built-in systems (in priority order)
    m_world->registerSystem<TransformSyncSystem>();
    m_world->registerSystem<RenderSyncSystem>();
    m_world->registerSystem<LightSystem>();
    m_world->registerSystem<EditorCameraSystem>();
    m_world->registerSystem<CameraSystem>();

    // User initialization
    onInit();

    FE_LOG_INFO("Entering main loop");

    // Main loop
    while (!m_window->shouldClose()) {
        // Update clock
        m_clock.tick();
        float dt = m_clock.getDeltaTime();

        // Poll events and update input
        m_window->pollEvents(m_input, m_eventBus);

        // Allow user to handle ESC to quit
        if (m_input.isKeyPressed(Key::Escape)) {
            break;
        }

        // User update
        onUpdate(dt);

        // ECS systems update (syncs to Filament)
        m_world->updateSystems(dt);

        // Render
        if (m_renderContext->beginFrame()) {
            m_renderContext->render();
            m_renderContext->endFrame();
        }
    }

    FE_LOG_INFO("Shutting down");

    // User cleanup
    onShutdown();

    // Destroy ECS world (cleans up entities and Filament components)
    m_world.reset();

    // Destroy resources
    resourceManager.reset();

    // Destroy render context
    m_renderContext.reset();

    // Destroy window
    m_window.reset();

    FE_LOG_INFO("Engine shutdown complete");
}

} // namespace fe
