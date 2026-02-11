#pragma once

#include <filament_engine/core/window.h>
#include <filament_engine/core/input.h>
#include <filament_engine/core/input_map.h>
#include <filament_engine/core/clock.h>
#include <filament_engine/core/event_bus.h>
#include <filament_engine/rendering/render_context.h>
#include <filament_engine/rendering/debug_renderer.h>
#include <filament_engine/ui/overlay.h>
#include <filament_engine/ui/imgui_layer.h>

#include <memory>
#include <string>
#include <vector>
#include <algorithm>

namespace fe {

class World; // forward declaration

struct ApplicationConfig {
    WindowConfig window;
    GraphicsBackend backend = GraphicsBackend::Vulkan;
};

// Subclass this and override onInit/onUpdate/onShutdown/onImGui.
class Application {
public:
    Application(const ApplicationConfig& config = {});
    virtual ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void run();
    virtual void onInit() {}
    virtual void onUpdate(float dt) {}
    virtual void onShutdown() {}
    virtual void onImGui() {} // override to draw ImGui widgets

    // Accessors
    Window& getWindow() { return *m_window; }
    RenderContext& getRenderContext() { return *m_renderContext; }
    Input& getInput() { return m_input; }
    InputMap& getInputMap() { return m_inputMap; }
    Clock& getClock() { return m_clock; }
    EventBus& getEventBus() { return m_eventBus; }
    World& getWorld() { return *m_world; }
    DebugRenderer& getDebugRenderer() { return *m_debugRenderer; }
    ImGuiLayer& getImGuiLayer() { return *m_imguiLayer; }

    // Overlay management
    template <typename T, typename... Args>
    T& addOverlay(Args&&... args) {
        auto overlay = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *overlay;
        m_overlays.push_back(std::move(overlay));
        std::sort(m_overlays.begin(), m_overlays.end(),
            [](const auto& a, const auto& b) { return a->priority < b->priority; });
        return ref;
    }

protected:
    ApplicationConfig m_config;

private:
    std::unique_ptr<Window> m_window;
    std::unique_ptr<RenderContext> m_renderContext;
    std::unique_ptr<World> m_world;
    std::unique_ptr<DebugRenderer> m_debugRenderer;
    std::unique_ptr<ImGuiLayer> m_imguiLayer;
    Input m_input;
    InputMap m_inputMap{"Default"};
    Clock m_clock;
    EventBus m_eventBus;
    std::vector<std::unique_ptr<Overlay>> m_overlays;
};

} // namespace fe
