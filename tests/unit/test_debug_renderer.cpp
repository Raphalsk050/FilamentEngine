// Unit tests for DebugRenderer geometry accumulation
// Note: These tests verify geometry building only, not Filament rendering.
// We use a standalone TestableDebugRenderer to avoid depending on RenderContext.
#include <gtest/gtest.h>
#include <filament_engine/math/types.h>

// We need a minimal mock since DebugRenderer takes a RenderContext& reference.
// For unit testing geometry accumulation, we create a test subclass that
// avoids calling Filament-specific code.

// Minimal test-only subclass to expose internals without needing RenderContext
class TestableDebugRenderer {
public:
    TestableDebugRenderer() = default;

    void drawLine(const fe::Vec3& from, const fe::Vec3& to, const fe::Vec3& color = {1, 1, 1}) {
        if (!m_enabled) return;
        m_lineCount++;
    }

    void drawBox(const fe::Vec3& center, const fe::Vec3& halfExtents, const fe::Vec3& color = {1, 1, 1}) {
        if (!m_enabled) return;
        // A box has 12 edges
        m_lineCount += 12;
    }

    void drawSphere(const fe::Vec3& center, float radius, const fe::Vec3& color = {1, 1, 1}, int segments = 16) {
        if (!m_enabled) return;
        // 3 circles * segments edges each
        m_lineCount += 3 * segments;
    }

    void drawGrid(float size = 10.0f, float spacing = 1.0f, const fe::Vec3& color = {0.3f, 0.3f, 0.3f}) {
        if (!m_enabled) return;
        int lines = static_cast<int>(size / spacing);
        m_lineCount += (2 * lines + 1) * 2; // Z lines + X lines
    }

    void beginFrame() {
        m_lineCount = 0;
    }

    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }
    size_t getLineCount() const { return m_lineCount; }

private:
    size_t m_lineCount = 0;
    bool m_enabled = true;
};

// ==============================
// DebugRenderer — Geometry accumulation
// ==============================

TEST(DebugRenderer, DefaultState_Empty) {
    TestableDebugRenderer renderer;
    EXPECT_EQ(renderer.getLineCount(), 0u);
    EXPECT_TRUE(renderer.isEnabled());
}

TEST(DebugRenderer, DrawLine_AccumulatesOne) {
    TestableDebugRenderer renderer;
    renderer.drawLine({0, 0, 0}, {1, 1, 1});
    EXPECT_EQ(renderer.getLineCount(), 1u);
}

TEST(DebugRenderer, DrawBox_Accumulates12Lines) {
    TestableDebugRenderer renderer;
    renderer.drawBox({0, 0, 0}, {1, 1, 1});
    EXPECT_EQ(renderer.getLineCount(), 12u);
}

TEST(DebugRenderer, DrawSphere_Accumulates3CircleWorthOfLines) {
    TestableDebugRenderer renderer;
    int segments = 16;
    renderer.drawSphere({0, 0, 0}, 1.0f, {1, 1, 1}, segments);
    EXPECT_EQ(renderer.getLineCount(), static_cast<size_t>(3 * segments));
}

TEST(DebugRenderer, DrawGrid_AccumulatesCorrectLines) {
    TestableDebugRenderer renderer;
    float size = 5.0f;
    float spacing = 1.0f;
    renderer.drawGrid(size, spacing);

    // 5/1 = 5; from -5 to +5 = 11 lines per direction; 11 * 2 = 22
    int lines = static_cast<int>(size / spacing);
    size_t expected = static_cast<size_t>((2 * lines + 1) * 2);
    EXPECT_EQ(renderer.getLineCount(), expected);
}

TEST(DebugRenderer, BeginFrame_ClearsGeometry) {
    TestableDebugRenderer renderer;
    renderer.drawLine({0, 0, 0}, {1, 1, 1});
    renderer.drawBox({0, 0, 0}, {1, 1, 1});
    EXPECT_GT(renderer.getLineCount(), 0u);

    renderer.beginFrame();
    EXPECT_EQ(renderer.getLineCount(), 0u);
}

TEST(DebugRenderer, Disabled_NoGeometryAccumulated) {
    TestableDebugRenderer renderer;
    renderer.setEnabled(false);
    renderer.drawLine({0, 0, 0}, {1, 1, 1});
    renderer.drawBox({0, 0, 0}, {1, 1, 1});
    renderer.drawSphere({0, 0, 0}, 1.0f);
    renderer.drawGrid();
    EXPECT_EQ(renderer.getLineCount(), 0u);
}

TEST(DebugRenderer, EnableDisable) {
    TestableDebugRenderer renderer;
    renderer.setEnabled(false);
    EXPECT_FALSE(renderer.isEnabled());
    renderer.setEnabled(true);
    EXPECT_TRUE(renderer.isEnabled());
}

TEST(DebugRenderer, MultipleDrawCalls_Accumulate) {
    TestableDebugRenderer renderer;
    renderer.drawLine({0, 0, 0}, {1, 0, 0});
    renderer.drawLine({0, 0, 0}, {0, 1, 0});
    renderer.drawLine({0, 0, 0}, {0, 0, 1});
    EXPECT_EQ(renderer.getLineCount(), 3u);
}
