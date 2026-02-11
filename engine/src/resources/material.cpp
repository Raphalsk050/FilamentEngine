#include <filament_engine/resources/material.h>
#include <filament_engine/core/log.h>

#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>

namespace fe {

MaterialWrapper MaterialWrapper::create(filament::Engine& engine, const void* data, size_t size) {
    MaterialWrapper wrapper;

    wrapper.m_material = filament::Material::Builder()
        .package(data, size)
        .build(engine);

    if (!wrapper.m_material) {
        FE_LOG_ERROR("Failed to create material from package");
        return wrapper;
    }

    wrapper.m_instance = wrapper.m_material->createInstance();
    return wrapper;
}

void MaterialWrapper::setBaseColor(const Vec4& color) {
    if (m_instance) {
        m_instance->setParameter("baseColor", filament::math::float4{color.x, color.y, color.z, color.w});
    }
}

void MaterialWrapper::setMetallic(float metallic) {
    if (m_instance) {
        m_instance->setParameter("metallic", metallic);
    }
}

void MaterialWrapper::setRoughness(float roughness) {
    if (m_instance) {
        m_instance->setParameter("roughness", roughness);
    }
}

void MaterialWrapper::setReflectance(float reflectance) {
    if (m_instance) {
        m_instance->setParameter("reflectance", reflectance);
    }
}

} // namespace fe
