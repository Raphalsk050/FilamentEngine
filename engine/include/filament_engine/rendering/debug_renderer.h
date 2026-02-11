#pragma once

#include <filament_engine/math/types.h>

#include <vector>
#include <cstdint>

namespace filament {
class Engine;
class VertexBuffer;
class IndexBuffer;
class Material;
class MaterialInstance;
class Entity;
} // namespace filament

namespace utils {
class Entity;
} // namespace utils

namespace fe {

class RenderContext;

// Immediate-mode debug renderer for visualizing lines, boxes, spheres, and grids.
// Commands are accumulated per frame and flushed to Filament as LINE primitives.
// Extremely lightweight — zero cost when disabled.
class DebugRenderer {
public:
    explicit DebugRenderer(RenderContext& renderContext);
    ~DebugRenderer();

    // Immediate-mode draw commands (accumulated per frame)
    void drawLine(const Vec3& from, const Vec3& to, const Vec3& color = {1, 1, 1});
    void drawBox(const Vec3& center, const Vec3& halfExtents, const Vec3& color = {1, 1, 1});
    void drawSphere(const Vec3& center, float radius, const Vec3& color = {1, 1, 1}, int segments = 16);
    void drawGrid(float size = 10.0f, float spacing = 1.0f, const Vec3& color = {0.3f, 0.3f, 0.3f});

    // Load the debug material from a compiled .filamat file
    void loadMaterial(const void* data, size_t size);

    // Frame lifecycle
    void beginFrame();  // clears previous frame's geometry
    void render();      // flushes accumulated geometry to Filament

    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

    // Query accumulated geometry (useful for testing)
    size_t getLineCount() const { return m_lines.size() / 2; }

private:
    struct DebugVertex {
        Vec3 position;
        uint32_t color; // packed RGBA
    };

    static uint32_t packColor(const Vec3& rgb);

    void cleanup();

    RenderContext& m_renderContext;
    std::vector<DebugVertex> m_lines; // pairs of vertices (start, end)
    bool m_enabled = true;

    // Filament resources (created/destroyed per frame)
    filament::Material* m_material = nullptr;
    filament::MaterialInstance* m_materialInstance = nullptr;
    filament::VertexBuffer* m_vertexBuffer = nullptr;
    filament::IndexBuffer* m_indexBuffer = nullptr;
    utils::Entity* m_renderable = nullptr;
    bool m_hasRenderable = false;
};

} // namespace fe
