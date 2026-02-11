#pragma once

#include <filament_engine/core/input.h>
#include <filament_engine/core/event_bus.h>

#include <string>
#include <cstdint>

struct SDL_Window;

namespace fe {

struct WindowConfig {
    std::string title = "Filament Engine";
    int width = 1280;
    int height = 720;
    bool resizable = true;
};

// RAII wrapper around an SDL2 window with platform-specific native handle extraction
class Window {
public:
    Window(const WindowConfig& config);
    ~Window();

    // Non-copyable
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // Poll SDL events, update input state, and fire events
    void pollEvents(Input& input, EventBus& eventBus);

    // Returns the native window handle for Filament SwapChain creation
    // On macOS with Vulkan: returns NSView*
    // On macOS with Metal: returns CAMetalLayer*
    void* getNativeWindow() const;

    // Returns the native window prepared for Vulkan rendering
    void* getNativeWindowForVulkan() const;

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    bool shouldClose() const { return m_shouldClose; }

    SDL_Window* getSDLWindow() const { return m_window; }

private:
    SDL_Window* m_window = nullptr;
    int m_width = 0;
    int m_height = 0;
    bool m_shouldClose = false;
};

} // namespace fe
