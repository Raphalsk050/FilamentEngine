#pragma once

#include <filament_engine/math/types.h>

#include <filament/Box.h>

namespace filament {
class Engine;
class VertexBuffer;
class IndexBuffer;
} // namespace filament

namespace fe {

// Represents a renderable mesh: vertex and index buffers with a bounding box
struct Mesh {
    filament::VertexBuffer* vertexBuffer = nullptr;
    filament::IndexBuffer* indexBuffer = nullptr;
    uint32_t indexCount = 0;
    filament::Box boundingBox;

    // Primitive geometry constructors
    static Mesh createCube(filament::Engine& engine, float halfExtent = 0.5f);
    static Mesh createPlane(filament::Engine& engine, float halfExtent = 1.0f);
};

} // namespace fe
