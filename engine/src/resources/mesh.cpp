#include <filament_engine/resources/mesh.h>

#include <filament/Engine.h>
#include <filament/VertexBuffer.h>
#include <filament/IndexBuffer.h>
#include <filament/RenderableManager.h>

#include <math/vec3.h>
#include <math/vec2.h>

#include <cstdint>
#include <vector>

namespace fe {

struct Vertex {
    filament::math::float3 position;
    filament::math::float3 normal;
    filament::math::float2 uv;
};

Mesh Mesh::createCube(filament::Engine& engine, float h) {
    // 24 vertices (4 per face for correct normals)
    static const Vertex vertices[] = {
        // Front face (+Z)
        {{-h, -h,  h}, { 0,  0,  1}, {0, 0}},
        {{ h, -h,  h}, { 0,  0,  1}, {1, 0}},
        {{ h,  h,  h}, { 0,  0,  1}, {1, 1}},
        {{-h,  h,  h}, { 0,  0,  1}, {0, 1}},
        // Back face (-Z)
        {{ h, -h, -h}, { 0,  0, -1}, {0, 0}},
        {{-h, -h, -h}, { 0,  0, -1}, {1, 0}},
        {{-h,  h, -h}, { 0,  0, -1}, {1, 1}},
        {{ h,  h, -h}, { 0,  0, -1}, {0, 1}},
        // Top face (+Y)
        {{-h,  h,  h}, { 0,  1,  0}, {0, 0}},
        {{ h,  h,  h}, { 0,  1,  0}, {1, 0}},
        {{ h,  h, -h}, { 0,  1,  0}, {1, 1}},
        {{-h,  h, -h}, { 0,  1,  0}, {0, 1}},
        // Bottom face (-Y)
        {{-h, -h, -h}, { 0, -1,  0}, {0, 0}},
        {{ h, -h, -h}, { 0, -1,  0}, {1, 0}},
        {{ h, -h,  h}, { 0, -1,  0}, {1, 1}},
        {{-h, -h,  h}, { 0, -1,  0}, {0, 1}},
        // Right face (+X)
        {{ h, -h,  h}, { 1,  0,  0}, {0, 0}},
        {{ h, -h, -h}, { 1,  0,  0}, {1, 0}},
        {{ h,  h, -h}, { 1,  0,  0}, {1, 1}},
        {{ h,  h,  h}, { 1,  0,  0}, {0, 1}},
        // Left face (-X)
        {{-h, -h, -h}, {-1,  0,  0}, {0, 0}},
        {{-h, -h,  h}, {-1,  0,  0}, {1, 0}},
        {{-h,  h,  h}, {-1,  0,  0}, {1, 1}},
        {{-h,  h, -h}, {-1,  0,  0}, {0, 1}},
    };

    static const uint16_t indices[] = {
         0,  1,  2,   2,  3,  0, // front
         4,  5,  6,   6,  7,  4, // back
         8,  9, 10,  10, 11,  8, // top
        12, 13, 14,  14, 15, 12, // bottom
        16, 17, 18,  18, 19, 16, // right
        20, 21, 22,  22, 23, 20, // left
    };

    constexpr uint32_t vertexCount = 24;
    constexpr uint32_t indexCount = 36;

    auto* vb = filament::VertexBuffer::Builder()
        .vertexCount(vertexCount)
        .bufferCount(1)
        .attribute(filament::VertexAttribute::POSITION,  0, filament::VertexBuffer::AttributeType::FLOAT3, offsetof(Vertex, position), sizeof(Vertex))
        .attribute(filament::VertexAttribute::TANGENTS,  0, filament::VertexBuffer::AttributeType::FLOAT3, offsetof(Vertex, normal),   sizeof(Vertex))
        .attribute(filament::VertexAttribute::UV0,       0, filament::VertexBuffer::AttributeType::FLOAT2, offsetof(Vertex, uv),       sizeof(Vertex))
        .build(engine);

    vb->setBufferAt(engine, 0,
        filament::VertexBuffer::BufferDescriptor(vertices, sizeof(vertices), nullptr));

    auto* ib = filament::IndexBuffer::Builder()
        .indexCount(indexCount)
        .bufferType(filament::IndexBuffer::IndexType::USHORT)
        .build(engine);

    ib->setBuffer(engine,
        filament::IndexBuffer::BufferDescriptor(indices, sizeof(indices), nullptr));

    Mesh mesh;
    mesh.vertexBuffer = vb;
    mesh.indexBuffer = ib;
    mesh.indexCount = indexCount;
    mesh.boundingBox = {{-h, -h, -h}, {h, h, h}};

    return mesh;
}

Mesh Mesh::createPlane(filament::Engine& engine, float h) {
    static const Vertex vertices[] = {
        {{-h, 0, -h}, {0, 1, 0}, {0, 0}},
        {{ h, 0, -h}, {0, 1, 0}, {1, 0}},
        {{ h, 0,  h}, {0, 1, 0}, {1, 1}},
        {{-h, 0,  h}, {0, 1, 0}, {0, 1}},
    };

    static const uint16_t indices[] = {
        0, 1, 2,  2, 3, 0
    };

    constexpr uint32_t vertexCount = 4;
    constexpr uint32_t indexCount = 6;

    auto* vb = filament::VertexBuffer::Builder()
        .vertexCount(vertexCount)
        .bufferCount(1)
        .attribute(filament::VertexAttribute::POSITION,  0, filament::VertexBuffer::AttributeType::FLOAT3, offsetof(Vertex, position), sizeof(Vertex))
        .attribute(filament::VertexAttribute::TANGENTS,  0, filament::VertexBuffer::AttributeType::FLOAT3, offsetof(Vertex, normal),   sizeof(Vertex))
        .attribute(filament::VertexAttribute::UV0,       0, filament::VertexBuffer::AttributeType::FLOAT2, offsetof(Vertex, uv),       sizeof(Vertex))
        .build(engine);

    vb->setBufferAt(engine, 0,
        filament::VertexBuffer::BufferDescriptor(vertices, sizeof(vertices), nullptr));

    auto* ib = filament::IndexBuffer::Builder()
        .indexCount(indexCount)
        .bufferType(filament::IndexBuffer::IndexType::USHORT)
        .build(engine);

    ib->setBuffer(engine,
        filament::IndexBuffer::BufferDescriptor(indices, sizeof(indices), nullptr));

    Mesh mesh;
    mesh.vertexBuffer = vb;
    mesh.indexBuffer = ib;
    mesh.indexCount = indexCount;
    mesh.boundingBox = {{-h, 0, -h}, {h, 0, h}};

    return mesh;
}

} // namespace fe
