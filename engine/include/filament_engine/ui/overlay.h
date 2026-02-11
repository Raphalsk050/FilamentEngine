#pragma once

#include <string>

namespace fe {

// Base class for screen-space overlays (stats, debug info, profiling, etc.)
// Overlays are drawn in priority order each frame when enabled.
class Overlay {
public:
    explicit Overlay(std::string name) : m_name(std::move(name)) {}
    virtual ~Overlay() = default;

    // Called each frame to draw this overlay (use ImGui or DebugRenderer)
    virtual void onDraw() = 0;

    const std::string& getName() const { return m_name; }

    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

    // Execution priority: lower values draw first (behind higher-priority overlays)
    int priority = 0;

private:
    std::string m_name;
    bool m_enabled = true;
};

} // namespace fe
