// Unit tests for EventBus (entt::dispatcher wrapper)
#include <gtest/gtest.h>
#include <filament_engine/core/event_bus.h>

// Custom event types for testing
struct TestEventA {
    int value = 0;
};

struct TestEventB {
    float x = 0.0f;
    float y = 0.0f;
};

// Receiver class for member function subscription tests
class EventReceiver {
public:
    void onTestEventA(const TestEventA& event) {
        lastValueA = event.value;
        countA++;
    }

    void onTestEventB(const TestEventB& event) {
        lastX = event.x;
        lastY = event.y;
        countB++;
    }

    int lastValueA = 0;
    int countA = 0;
    float lastX = 0.0f;
    float lastY = 0.0f;
    int countB = 0;
};

// =====================
// Publish (immediate trigger)
// =====================

TEST(EventBus, Publish_TriggersMemberSubscriber) {
    fe::EventBus bus;
    EventReceiver receiver;

    bus.subscribe<TestEventA, &EventReceiver::onTestEventA>(receiver);
    bus.publish(TestEventA{42});

    EXPECT_EQ(receiver.lastValueA, 42);
    EXPECT_EQ(receiver.countA, 1);
}

TEST(EventBus, Publish_MultipleEvents) {
    fe::EventBus bus;
    EventReceiver receiver;

    bus.subscribe<TestEventA, &EventReceiver::onTestEventA>(receiver);

    bus.publish(TestEventA{10});
    bus.publish(TestEventA{20});
    bus.publish(TestEventA{30});

    EXPECT_EQ(receiver.lastValueA, 30);
    EXPECT_EQ(receiver.countA, 3);
}

// =====================
// Enqueue + Update (deferred dispatch)
// =====================

TEST(EventBus, Enqueue_DoesNotTriggerImmediately) {
    fe::EventBus bus;
    EventReceiver receiver;

    bus.subscribe<TestEventA, &EventReceiver::onTestEventA>(receiver);
    bus.enqueue(TestEventA{99});

    EXPECT_EQ(receiver.countA, 0); // not yet dispatched
}

TEST(EventBus, Enqueue_DispatchesOnUpdate) {
    fe::EventBus bus;
    EventReceiver receiver;

    bus.subscribe<TestEventA, &EventReceiver::onTestEventA>(receiver);
    bus.enqueue(TestEventA{99});
    bus.update();

    EXPECT_EQ(receiver.lastValueA, 99);
    EXPECT_EQ(receiver.countA, 1);
}

TEST(EventBus, Enqueue_MultipleEvents_DispatchedOnUpdate) {
    fe::EventBus bus;
    EventReceiver receiver;

    bus.subscribe<TestEventA, &EventReceiver::onTestEventA>(receiver);
    bus.enqueue(TestEventA{1});
    bus.enqueue(TestEventA{2});
    bus.enqueue(TestEventA{3});

    EXPECT_EQ(receiver.countA, 0);
    bus.update();
    EXPECT_EQ(receiver.countA, 3);
    EXPECT_EQ(receiver.lastValueA, 3);
}

// =====================
// Unsubscribe
// =====================

TEST(EventBus, Unsubscribe_StopsReceiving) {
    fe::EventBus bus;
    EventReceiver receiver;

    bus.subscribe<TestEventA, &EventReceiver::onTestEventA>(receiver);
    bus.publish(TestEventA{1});
    EXPECT_EQ(receiver.countA, 1);

    bus.unsubscribe<TestEventA, &EventReceiver::onTestEventA>(receiver);
    bus.publish(TestEventA{2});
    EXPECT_EQ(receiver.countA, 1); // no additional call
    EXPECT_EQ(receiver.lastValueA, 1);
}

// =====================
// Multiple event types
// =====================

TEST(EventBus, MultipleEventTypes_Independent) {
    fe::EventBus bus;
    EventReceiver receiver;

    bus.subscribe<TestEventA, &EventReceiver::onTestEventA>(receiver);
    bus.subscribe<TestEventB, &EventReceiver::onTestEventB>(receiver);

    bus.publish(TestEventA{100});
    EXPECT_EQ(receiver.countA, 1);
    EXPECT_EQ(receiver.countB, 0);

    bus.publish(TestEventB{1.5f, 2.5f});
    EXPECT_EQ(receiver.countA, 1);
    EXPECT_EQ(receiver.countB, 1);
    EXPECT_FLOAT_EQ(receiver.lastX, 1.5f);
    EXPECT_FLOAT_EQ(receiver.lastY, 2.5f);
}

// =====================
// Built-in event types
// =====================

TEST(EventBus, WindowResizeEvent) {
    fe::EventBus bus;
    int capturedWidth = 0;
    int capturedHeight = 0;

    struct ResizeReceiver {
        int& w;
        int& h;
        void onResize(const fe::WindowResizeEvent& e) {
            w = e.width;
            h = e.height;
        }
    } recv{capturedWidth, capturedHeight};

    bus.subscribe<fe::WindowResizeEvent, &ResizeReceiver::onResize>(recv);
    bus.publish(fe::WindowResizeEvent{1920, 1080});

    EXPECT_EQ(capturedWidth, 1920);
    EXPECT_EQ(capturedHeight, 1080);
}

TEST(EventBus, Update_ClearsQueue) {
    fe::EventBus bus;
    EventReceiver receiver;

    bus.subscribe<TestEventA, &EventReceiver::onTestEventA>(receiver);
    bus.enqueue(TestEventA{42});
    bus.update();

    // Second update should not dispatch again
    bus.update();
    EXPECT_EQ(receiver.countA, 1);
}
