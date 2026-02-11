#pragma once

#include <string>

namespace fe {

class RenderContext;
class Window;

// ImGui integration layer for Filament Engine.
// Manages the ImGui lifecycle, event forwarding, and rendering via Filament's filagui.
//
// Usage within Application:
//   void onImGui() override {
//       ImGui::Begin("Debug");
//       ImGui::Text("FPS: %.1f", getClock().getFPS());
//       ImGui::End();
//   }
class ImGuiLayer {
public:
    ImGuiLayer(RenderContext& renderContext, Window& window);
    ~ImGuiLayer();

    // Frame lifecycle
    void beginFrame(float dt);
    void endFrame();

    // Query whether ImGui wants to capture input
    bool wantsCaptureMouse() const { return m_wantsCaptureMouse; }
    bool wantsCaptureKeyboard() const { return m_wantsCaptureKeyboard; }

    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

private:
    RenderContext& m_renderContext;
    Window& m_window;
    bool m_enabled = true;
    bool m_initialized = false;
    bool m_wantsCaptureMouse = false;
    bool m_wantsCaptureKeyboard = false;
};

} // namespace fe
