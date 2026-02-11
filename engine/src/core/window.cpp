#include <filament_engine/core/window.h>
#include <filament_engine/core/log.h>

#include <SDL.h>
#include <SDL_syswm.h>

namespace fe {

Window::Window(const WindowConfig& config)
    : m_width(config.width)
    , m_height(config.height) {

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        FE_LOG_FATAL("Failed to initialize SDL: %s", SDL_GetError());
        return;
    }

    uint32_t flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
    if (config.resizable) {
        flags |= SDL_WINDOW_RESIZABLE;
    }

    // For Vulkan, we need to use SDL_WINDOW_VULKAN
    flags |= SDL_WINDOW_VULKAN;

    m_window = SDL_CreateWindow(
        config.title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        config.width,
        config.height,
        flags
    );

    if (!m_window) {
        // Fallback without Vulkan flag
        flags &= ~SDL_WINDOW_VULKAN;
        m_window = SDL_CreateWindow(
            config.title.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            config.width,
            config.height,
            flags
        );
    }

    if (!m_window) {
        FE_LOG_FATAL("Failed to create SDL window: %s", SDL_GetError());
        return;
    }

    // Get actual drawable size (HiDPI aware)
    SDL_GL_GetDrawableSize(m_window, &m_width, &m_height);
    FE_LOG_INFO("Window created: %dx%d", m_width, m_height);
}

Window::~Window() {
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    SDL_Quit();
}

void Window::pollEvents(Input& input, EventBus& eventBus) {
    input.beginFrame();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                m_shouldClose = true;
                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
                    event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    m_width = event.window.data1;
                    m_height = event.window.data2;
                    eventBus.publish(WindowResizeEvent{m_width, m_height});
                }
                break;

            case SDL_KEYDOWN:
                input.onKeyEvent(event.key.keysym.scancode, true);
                eventBus.publish(KeyEvent{
                    event.key.keysym.scancode,
                    event.key.keysym.sym,
                    true,
                    event.key.repeat != 0
                });
                break;

            case SDL_KEYUP:
                input.onKeyEvent(event.key.keysym.scancode, false);
                eventBus.publish(KeyEvent{
                    event.key.keysym.scancode,
                    event.key.keysym.sym,
                    false,
                    false
                });
                break;

            case SDL_MOUSEMOTION:
                input.onMouseMove(
                    static_cast<float>(event.motion.x),
                    static_cast<float>(event.motion.y),
                    static_cast<float>(event.motion.xrel),
                    static_cast<float>(event.motion.yrel)
                );
                eventBus.publish(MouseMoveEvent{
                    static_cast<float>(event.motion.x),
                    static_cast<float>(event.motion.y),
                    static_cast<float>(event.motion.xrel),
                    static_cast<float>(event.motion.yrel)
                });
                break;

            case SDL_MOUSEBUTTONDOWN:
                input.onMouseButton(event.button.button, true);
                eventBus.publish(MouseButtonEvent{
                    event.button.button,
                    true,
                    static_cast<float>(event.button.x),
                    static_cast<float>(event.button.y)
                });
                break;

            case SDL_MOUSEBUTTONUP:
                input.onMouseButton(event.button.button, false);
                eventBus.publish(MouseButtonEvent{
                    event.button.button,
                    false,
                    static_cast<float>(event.button.x),
                    static_cast<float>(event.button.y)
                });
                break;

            case SDL_MOUSEWHEEL:
                input.onMouseScroll(
                    static_cast<float>(event.wheel.x),
                    static_cast<float>(event.wheel.y)
                );
                eventBus.publish(MouseScrollEvent{
                    static_cast<float>(event.wheel.x),
                    static_cast<float>(event.wheel.y)
                });
                break;
        }
    }
}

void* Window::getNativeWindow() const {
    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    if (!SDL_GetWindowWMInfo(m_window, &wmi)) {
        FE_LOG_ERROR("Failed to get native window info: %s", SDL_GetError());
        return nullptr;
    }

#if defined(__APPLE__)
    // On macOS, return the NSView* for Cocoa
    return wmi.info.cocoa.window;
#elif defined(__linux__)
    return (void*)(uintptr_t)wmi.info.x11.window;
#elif defined(_WIN32)
    return wmi.info.win.window;
#else
    return nullptr;
#endif
}

void* Window::getNativeWindowForVulkan() const {
    // For Vulkan on macOS, Filament needs the NSView*
    // The VulkanPlatform will create the VkSurfaceKHR from it
    return getNativeWindow();
}

} // namespace fe
