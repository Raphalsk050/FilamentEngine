#include <filament_engine/rendering/debug_renderer.h>
#include <filament_engine/rendering/render_context.h>
#include <filament_engine/core/log.h>

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace fe {

DebugRenderer::DebugRenderer(RenderContext& renderContext)
    : m_renderContext(renderContext) {
    m_lines.reserve(4096); // pre-allocate for typical debug scene
    FE_LOG_DEBUG("DebugRenderer created");
}

DebugRenderer::~DebugRenderer() {
    FE_LOG_DEBUG("DebugRenderer destroyed");
}

void DebugRenderer::beginFrame() {
    m_lines.clear();
}

void DebugRenderer::drawLine(const Vec3& from, const Vec3& to, const Vec3& color) {
    if (!m_enabled) return;
    m_lines.push_back({from, color});
    m_lines.push_back({to, color});
}

void DebugRenderer::drawBox(const Vec3& center, const Vec3& half, const Vec3& color) {
    if (!m_enabled) return;

    // 12 edges of a box
    Vec3 corners[8] = {
        {center.x - half.x, center.y - half.y, center.z - half.z},
        {center.x + half.x, center.y - half.y, center.z - half.z},
        {center.x + half.x, center.y + half.y, center.z - half.z},
        {center.x - half.x, center.y + half.y, center.z - half.z},
        {center.x - half.x, center.y - half.y, center.z + half.z},
        {center.x + half.x, center.y - half.y, center.z + half.z},
        {center.x + half.x, center.y + half.y, center.z + half.z},
        {center.x - half.x, center.y + half.y, center.z + half.z},
    };

    // Bottom face
    drawLine(corners[0], corners[1], color);
    drawLine(corners[1], corners[2], color);
    drawLine(corners[2], corners[3], color);
    drawLine(corners[3], corners[0], color);
    // Top face
    drawLine(corners[4], corners[5], color);
    drawLine(corners[5], corners[6], color);
    drawLine(corners[6], corners[7], color);
    drawLine(corners[7], corners[4], color);
    // Vertical edges
    drawLine(corners[0], corners[4], color);
    drawLine(corners[1], corners[5], color);
    drawLine(corners[2], corners[6], color);
    drawLine(corners[3], corners[7], color);
}

void DebugRenderer::drawSphere(const Vec3& center, float radius, const Vec3& color, int segments) {
    if (!m_enabled) return;

    // Draw 3 circles (XY, XZ, YZ planes)
    for (int i = 0; i < segments; ++i) {
        float a0 = (2.0f * static_cast<float>(M_PI) * i) / segments;
        float a1 = (2.0f * static_cast<float>(M_PI) * (i + 1)) / segments;

        float c0 = std::cos(a0) * radius;
        float s0 = std::sin(a0) * radius;
        float c1 = std::cos(a1) * radius;
        float s1 = std::sin(a1) * radius;

        // XY circle
        drawLine({center.x + c0, center.y + s0, center.z},
                 {center.x + c1, center.y + s1, center.z}, color);
        // XZ circle
        drawLine({center.x + c0, center.y, center.z + s0},
                 {center.x + c1, center.y, center.z + s1}, color);
        // YZ circle
        drawLine({center.x, center.y + c0, center.z + s0},
                 {center.x, center.y + c1, center.z + s1}, color);
    }
}

void DebugRenderer::drawGrid(float size, float spacing, const Vec3& color) {
    if (!m_enabled) return;

    int lines = static_cast<int>(size / spacing);
    for (int i = -lines; i <= lines; ++i) {
        float offset = i * spacing;
        // Lines along Z
        drawLine({offset, 0, -size}, {offset, 0, size}, color);
        // Lines along X
        drawLine({-size, 0, offset}, {size, 0, offset}, color);
    }
}

void DebugRenderer::render() {
    if (!m_enabled || m_lines.empty()) return;

    // TODO: Flush debug lines to Filament using a simple unlit material.
    // For now, geometry is accumulated and ready for rendering.
    // A future implementation will use Filament's DebugRegistry or
    // create a custom renderable with line primitives.
}

} // namespace fe
