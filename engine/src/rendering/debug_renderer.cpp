#include <filament_engine/rendering/debug_renderer.h>
#include <filament_engine/rendering/render_context.h>
#include <filament_engine/core/log.h>

#include <filament/Engine.h>
#include <filament/VertexBuffer.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Box.h>
#include <utils/EntityManager.h>

#include <cmath>
#include <cstring>

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
    cleanup();
    if (m_materialInstance) {
        m_renderContext.getEngine()->destroy(m_materialInstance);
        m_materialInstance = nullptr;
    }
    if (m_material) {
        m_renderContext.getEngine()->destroy(m_material);
        m_material = nullptr;
    }
    FE_LOG_DEBUG("DebugRenderer destroyed");
}

uint32_t DebugRenderer::packColor(const Vec3& rgb) {
    auto r = static_cast<uint8_t>(std::clamp(rgb.x, 0.0f, 1.0f) * 255.0f);
    auto g = static_cast<uint8_t>(std::clamp(rgb.y, 0.0f, 1.0f) * 255.0f);
    auto b = static_cast<uint8_t>(std::clamp(rgb.z, 0.0f, 1.0f) * 255.0f);
    uint8_t a = 255;
    return (a << 24) | (b << 16) | (g << 8) | r; // ABGR packed
}

void DebugRenderer::loadMaterial(const void* data, size_t size) {
    auto* engine = m_renderContext.getEngine();
    m_material = filament::Material::Builder()
        .package(data, size)
        .build(*engine);

    if (m_material) {
        m_materialInstance = m_material->createInstance();
        FE_LOG_INFO("DebugRenderer material loaded");
    } else {
        FE_LOG_ERROR("DebugRenderer: failed to load material");
    }
}

void DebugRenderer::beginFrame() {
    cleanup();
    m_lines.clear();
}

void DebugRenderer::drawLine(const Vec3& from, const Vec3& to, const Vec3& color) {
    if (!m_enabled) return;
    uint32_t packed = packColor(color);
    m_lines.push_back({from, packed});
    m_lines.push_back({to, packed});
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
        float offset = static_cast<float>(i) * spacing;
        // Lines along Z
        drawLine({offset, 0, -size}, {offset, 0, size}, color);
        // Lines along X
        drawLine({-size, 0, offset}, {size, 0, offset}, color);
    }
}

void DebugRenderer::cleanup() {
    auto* engine = m_renderContext.getEngine();
    auto* scene = m_renderContext.getScene();

    if (m_hasRenderable && m_renderable) {
        scene->remove(*reinterpret_cast<utils::Entity*>(m_renderable));
        engine->destroy(*reinterpret_cast<utils::Entity*>(m_renderable));
        utils::EntityManager::get().destroy(*reinterpret_cast<utils::Entity*>(m_renderable));
        delete m_renderable;
        m_renderable = nullptr;
        m_hasRenderable = false;
    }

    if (m_vertexBuffer) {
        engine->destroy(m_vertexBuffer);
        m_vertexBuffer = nullptr;
    }
    if (m_indexBuffer) {
        engine->destroy(m_indexBuffer);
        m_indexBuffer = nullptr;
    }
}

void DebugRenderer::render() {
    if (!m_enabled || m_lines.empty() || !m_materialInstance) return;

    auto* engine = m_renderContext.getEngine();
    auto* scene = m_renderContext.getScene();

    uint32_t vertexCount = static_cast<uint32_t>(m_lines.size());
    uint32_t indexCount = vertexCount; // 1:1 for lines

    // Create vertex buffer with position + color
    m_vertexBuffer = filament::VertexBuffer::Builder()
        .vertexCount(vertexCount)
        .bufferCount(1)
        .attribute(filament::VertexAttribute::POSITION, 0,
            filament::VertexBuffer::AttributeType::FLOAT3,
            offsetof(DebugVertex, position), sizeof(DebugVertex))
        .attribute(filament::VertexAttribute::COLOR, 0,
            filament::VertexBuffer::AttributeType::UBYTE4,
            offsetof(DebugVertex, color), sizeof(DebugVertex))
        .normalized(filament::VertexAttribute::COLOR)
        .build(*engine);

    // Copy vertex data — Filament takes ownership via callback
    auto* vertexData = new DebugVertex[vertexCount];
    std::memcpy(vertexData, m_lines.data(), vertexCount * sizeof(DebugVertex));

    m_vertexBuffer->setBufferAt(*engine, 0,
        filament::VertexBuffer::BufferDescriptor(
            vertexData, vertexCount * sizeof(DebugVertex),
            [](void* buffer, size_t, void*) { delete[] static_cast<DebugVertex*>(buffer); }));

    // Create index buffer (sequential indices for line pairs)
    m_indexBuffer = filament::IndexBuffer::Builder()
        .indexCount(indexCount)
        .bufferType(filament::IndexBuffer::IndexType::USHORT)
        .build(*engine);

    auto* indexData = new uint16_t[indexCount];
    for (uint32_t i = 0; i < indexCount; ++i) {
        indexData[i] = static_cast<uint16_t>(i);
    }

    m_indexBuffer->setBuffer(*engine,
        filament::IndexBuffer::BufferDescriptor(
            indexData, indexCount * sizeof(uint16_t),
            [](void* buffer, size_t, void*) { delete[] static_cast<uint16_t*>(buffer); }));

    // Create a renderable entity with LINES primitive type
    auto entity = utils::EntityManager::get().create();
    m_renderable = new utils::Entity(entity);
    m_hasRenderable = true;

    // Compute bounding box from all vertices
    Vec3 bMin = m_lines[0].position;
    Vec3 bMax = m_lines[0].position;
    for (const auto& v : m_lines) {
        bMin.x = std::min(bMin.x, v.position.x);
        bMin.y = std::min(bMin.y, v.position.y);
        bMin.z = std::min(bMin.z, v.position.z);
        bMax.x = std::max(bMax.x, v.position.x);
        bMax.y = std::max(bMax.y, v.position.y);
        bMax.z = std::max(bMax.z, v.position.z);
    }

    filament::RenderableManager::Builder(1)
        .boundingBox({{bMin.x, bMin.y, bMin.z}, {bMax.x, bMax.y, bMax.z}})
        .material(0, m_materialInstance)
        .geometry(0, filament::RenderableManager::PrimitiveType::LINES,
            m_vertexBuffer, m_indexBuffer, 0, indexCount)
        .culling(false)
        .receiveShadows(false)
        .castShadows(false)
        .build(*engine, entity);

    scene->addEntity(entity);
}

} // namespace fe
