#pragma once

#include <entt/entt.hpp>

namespace fe {

// Built-in event types
struct WindowResizeEvent {
    int width;
    int height;
};

struct KeyEvent {
    int scancode;
    int keycode;
    bool pressed; // true = pressed, false = released
    bool repeat;
};

struct MouseMoveEvent {
    float x;
    float y;
    float deltaX;
    float deltaY;
};

struct MouseButtonEvent {
    int button;
    bool pressed;
    float x;
    float y;
};

struct MouseScrollEvent {
    float xOffset;
    float yOffset;
};

// Lightweight event bus wrapping entt::dispatcher
class EventBus {
public:
    // Publish an event to all subscribers
    template <typename T>
    void publish(const T& event) {
        m_dispatcher.trigger(event);
    }

    // Enqueue an event to be dispatched later via update()
    template <typename T>
    void enqueue(const T& event) {
        m_dispatcher.enqueue(event);
    }

    // Subscribe to an event type
    template <typename T, auto Func>
    void subscribe(entt::id_type id = entt::type_hash<T>::value()) {
        m_dispatcher.sink<T>().template connect<Func>();
    }

    // Subscribe a member function
    template <typename T, auto Func, typename Instance>
    void subscribe(Instance& instance) {
        m_dispatcher.sink<T>().template connect<Func>(instance);
    }

    // Unsubscribe a member function
    template <typename T, auto Func, typename Instance>
    void unsubscribe(Instance& instance) {
        m_dispatcher.sink<T>().template disconnect<Func>(instance);
    }

    // Dispatch all queued events
    void update() {
        m_dispatcher.update();
    }

private:
    entt::dispatcher m_dispatcher;
};

} // namespace fe
