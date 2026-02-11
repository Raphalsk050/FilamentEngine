#include <filament_engine/resources/resource_manager.h>
#include <filament_engine/core/log.h>

#include <filament/Engine.h>
#include <filament/VertexBuffer.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>

namespace fe {

ResourceManager* ResourceManager::s_instance = nullptr;

ResourceManager::ResourceManager(filament::Engine& engine)
    : m_engine(engine) {
    s_instance = this;
    FE_LOG_INFO("ResourceManager created");
}

ResourceManager::~ResourceManager() {
    destroyAll();
    if (s_instance == this) {
        s_instance = nullptr;
    }
    FE_LOG_INFO("ResourceManager destroyed");
}

ResourceHandle<Mesh> ResourceManager::addMesh(Mesh mesh) {
    uint32_t id = m_nextId++;
    m_meshes[id] = mesh;
    return ResourceHandle<Mesh>(id);
}

Mesh* ResourceManager::getMesh(ResourceHandle<Mesh> handle) {
    auto it = m_meshes.find(handle.getId());
    return (it != m_meshes.end()) ? &it->second : nullptr;
}

ResourceHandle<MaterialWrapper> ResourceManager::addMaterial(MaterialWrapper material) {
    uint32_t id = m_nextId++;
    m_materials[id] = material;
    return ResourceHandle<MaterialWrapper>(id);
}

MaterialWrapper* ResourceManager::getMaterial(ResourceHandle<MaterialWrapper> handle) {
    auto it = m_materials.find(handle.getId());
    return (it != m_materials.end()) ? &it->second : nullptr;
}

ResourceHandle<MaterialWrapper> ResourceManager::createMaterial(const void* data, size_t size) {
    auto material = MaterialWrapper::create(m_engine, data, size);
    if (!material.isValid()) {
        return ResourceHandle<MaterialWrapper>{};
    }
    return addMaterial(material);
}

void ResourceManager::destroyAll() {
    for (auto& [id, mesh] : m_meshes) {
        if (mesh.vertexBuffer) m_engine.destroy(mesh.vertexBuffer);
        if (mesh.indexBuffer) m_engine.destroy(mesh.indexBuffer);
    }
    m_meshes.clear();

    for (auto& [id, material] : m_materials) {
        if (material.getInstance()) m_engine.destroy(material.getInstance());
        if (material.getMaterial()) m_engine.destroy(material.getMaterial());
    }
    m_materials.clear();

    FE_LOG_INFO("All resources destroyed");
}

} // namespace fe
