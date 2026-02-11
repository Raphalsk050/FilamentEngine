#pragma once

#include <filament_engine/core/window.h>
#include <filament_engine/core/input.h>
#include <filament_engine/core/clock.h>
#include <filament_engine/core/event_bus.h>
#include <filament_engine/rendering/render_context.h>

#include <memory>
#include <string>

namespace fe {

class World; // forward declaration

struct ApplicationConfig {
    WindowConfig window;
    GraphicsBackend backend = GraphicsBackend::Vulkan;
};

// Base class for engine applications.
// Users subclass this and override onInit/onUpdate/onShutdown to build their game.
class Application {
public:
    Application(const ApplicationConfig& config = {});
    virtual ~Application();

    // Non-copyable
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // Main entry point: runs the game loop until window is closed
    void run();

    // Override these in your application
    virtual void onInit() {}
    virtual void onUpdate(float dt) {}
    virtual void onShutdown() {}

    // Access engine subsystems
    Window& getWindow() { return *m_window; }
    RenderContext& getRenderContext() { return *m_renderContext; }
    Input& getInput() { return m_input; }
    Clock& getClock() { return m_clock; }
    EventBus& getEventBus() { return m_eventBus; }
    World& getWorld() { return *m_world; }

protected:
    ApplicationConfig m_config;

private:
    std::unique_ptr<Window> m_window;
    std::unique_ptr<RenderContext> m_renderContext;
    std::unique_ptr<World> m_world;
    Input m_input;
    Clock m_clock;
    EventBus m_eventBus;
};

} // namespace fe
