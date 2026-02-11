// Unit tests for Overlay base class
#include <gtest/gtest.h>
#include <filament_engine/ui/overlay.h>

#include <vector>
#include <algorithm>

// Concrete overlay for testing
class TestOverlay : public fe::Overlay {
public:
    using fe::Overlay::Overlay;

    void onDraw() override {
        drawCount++;
    }

    int drawCount = 0;
};

TEST(Overlay, DefaultEnabled) {
    TestOverlay overlay("Test");
    EXPECT_TRUE(overlay.isEnabled());
    EXPECT_EQ(overlay.getName(), "Test");
}

TEST(Overlay, EnableDisable) {
    TestOverlay overlay("Test");
    overlay.setEnabled(false);
    EXPECT_FALSE(overlay.isEnabled());
    overlay.setEnabled(true);
    EXPECT_TRUE(overlay.isEnabled());
}

TEST(Overlay, OnDraw_Called) {
    TestOverlay overlay("Test");
    overlay.onDraw();
    overlay.onDraw();
    EXPECT_EQ(overlay.drawCount, 2);
}

TEST(Overlay, DefaultPriority) {
    TestOverlay overlay("Test");
    EXPECT_EQ(overlay.priority, 0);
}

TEST(Overlay, PriorityOrdering) {
    std::vector<std::unique_ptr<fe::Overlay>> overlays;
    auto high = std::make_unique<TestOverlay>("High");
    high->priority = 10;
    auto low = std::make_unique<TestOverlay>("Low");
    low->priority = 1;
    auto mid = std::make_unique<TestOverlay>("Mid");
    mid->priority = 5;

    overlays.push_back(std::move(high));
    overlays.push_back(std::move(low));
    overlays.push_back(std::move(mid));

    // Sort by priority
    std::sort(overlays.begin(), overlays.end(),
        [](const auto& a, const auto& b) { return a->priority < b->priority; });

    EXPECT_EQ(overlays[0]->getName(), "Low");
    EXPECT_EQ(overlays[1]->getName(), "Mid");
    EXPECT_EQ(overlays[2]->getName(), "High");
}

TEST(Overlay, DisabledOverlay_SkippedInDispatch) {
    std::vector<std::unique_ptr<TestOverlay>> overlays;
    auto a = std::make_unique<TestOverlay>("A");
    auto b = std::make_unique<TestOverlay>("B");
    b->setEnabled(false);
    auto c = std::make_unique<TestOverlay>("C");

    overlays.push_back(std::move(a));
    overlays.push_back(std::move(b));
    overlays.push_back(std::move(c));

    // Simulate dispatch loop
    for (auto& overlay : overlays) {
        if (overlay->isEnabled()) {
            overlay->onDraw();
        }
    }

    EXPECT_EQ(overlays[0]->drawCount, 1);
    EXPECT_EQ(overlays[1]->drawCount, 0); // disabled
    EXPECT_EQ(overlays[2]->drawCount, 1);
}
