#include <filament_engine/ui/imgui_layer.h>
#include <filament_engine/rendering/render_context.h>
#include <filament_engine/core/window.h>
#include <filament_engine/core/log.h>

namespace fe {

ImGuiLayer::ImGuiLayer(RenderContext& renderContext, Window& window)
    : m_renderContext(renderContext), m_window(window) {
    // TODO: Initialize filagui::ImGuiHelper with Filament's Engine, View, and Window.
    // This requires linking against the filagui library from Filament's distribution.
    // The implementation will:
    //   1. Create an ImGuiHelper instance
    //   2. Configure ImGui context (fonts, style, DPI scaling)
    //   3. Set up the Filament View overlay for ImGui rendering
    FE_LOG_INFO("ImGuiLayer created (stub — awaiting filagui linkage)");
}

ImGuiLayer::~ImGuiLayer() {
    // TODO: Destroy ImGuiHelper and cleanup ImGui context
    FE_LOG_INFO("ImGuiLayer destroyed");
}

void ImGuiLayer::beginFrame(float dt) {
    if (!m_enabled) return;
    // TODO: Call ImGuiHelper::setFrameDelta(dt) and process events
}

void ImGuiLayer::endFrame() {
    if (!m_enabled) return;
    // TODO: Call ImGuiHelper::render() to submit ImGui draw data to Filament
    // After rendering, query ImGui's IO for capture state
    m_wantsCaptureMouse = false;     // TODO: ImGui::GetIO().WantCaptureMouse
    m_wantsCaptureKeyboard = false;  // TODO: ImGui::GetIO().WantCaptureKeyboard
}

} // namespace fe
