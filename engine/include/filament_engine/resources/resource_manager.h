#pragma once

#include <filament_engine/resources/resource_handle.h>
#include <filament_engine/resources/mesh.h>
#include <filament_engine/resources/material.h>

#include <unordered_map>
#include <cstdint>

namespace filament {
class Engine;
} // namespace filament

namespace fe {

// Manages GPU resources (meshes, materials) with handle-based access.
// Singleton pattern: one instance per engine lifetime.
class ResourceManager {
public:
    explicit ResourceManager(filament::Engine& engine);
    ~ResourceManager();

    // Non-copyable
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    // Singleton access (set when Application creates ResourceManager)
    static ResourceManager* getInstance() { return s_instance; }

    // Mesh management
    ResourceHandle<Mesh> addMesh(Mesh mesh);
    Mesh* getMesh(ResourceHandle<Mesh> handle);

    // Material management
    ResourceHandle<MaterialWrapper> addMaterial(MaterialWrapper material);
    MaterialWrapper* getMaterial(ResourceHandle<MaterialWrapper> handle);

    // Create material from compiled package data
    ResourceHandle<MaterialWrapper> createMaterial(const void* data, size_t size);

    // Cleanup all resources
    void destroyAll();

private:
    filament::Engine& m_engine;
    uint32_t m_nextId = 1; // 0 is reserved for invalid

    std::unordered_map<uint32_t, Mesh> m_meshes;
    std::unordered_map<uint32_t, MaterialWrapper> m_materials;

    static ResourceManager* s_instance;
};

} // namespace fe
