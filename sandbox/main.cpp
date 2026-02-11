#include <filament_engine/filament_engine.h>

#include <cmath>
#include <fstream>
#include <vector>
static std::vector<uint8_t> loadFile(const char* path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        FE_LOG_ERROR("Failed to open file: %s", path);
        return {};
    }
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    return data;
}

class HelloCubeApp : public fe::Application {
public:
    HelloCubeApp() : fe::Application(makeConfig()) {}

    void onInit() override {
        auto& world = getWorld();
        auto& renderCtx = getRenderContext();
        auto* resourceMgr = fe::ResourceManager::getInstance();
        renderCtx.loadIBL("assets/ibl");
        auto cubeMesh = fe::Mesh::createCube(*renderCtx.getEngine(), 0.5f);
        m_cubeMeshHandle = resourceMgr->addMesh(cubeMesh);
        auto materialData = loadFile("materials/standard_lit.filamat");
        if (materialData.empty()) {
            FE_LOG_FATAL("Could not load standard_lit.filamat material");
            return;
        }
        m_materialHandle = resourceMgr->createMaterial(materialData.data(), materialData.size());
        auto* material = resourceMgr->getMaterial(m_materialHandle);
        if (material) {
            material->setBaseColor({0.8f, 0.2f, 0.2f, 1.0f}); // red
            material->setMetallic(0.0f);
            material->setRoughness(0.4f);
            material->setReflectance(0.5f);
        }
        m_cubeEntity = world.createEntity("Cube");
        auto& meshRenderer = world.addComponent<fe::MeshRendererComponent>(m_cubeEntity);
        meshRenderer.mesh = m_cubeMeshHandle;
        meshRenderer.material = m_materialHandle;
        meshRenderer.castShadows = true;
        meshRenderer.receiveShadows = true;
        auto planeMesh = fe::Mesh::createCube(*renderCtx.getEngine(), 5.0f);
        m_planeMeshHandle = resourceMgr->addMesh(planeMesh);
        auto planeMaterialData = loadFile("materials/standard_lit.filamat");
        m_planeMaterialHandle = resourceMgr->createMaterial(planeMaterialData.data(), planeMaterialData.size());
        auto* planeMaterial = resourceMgr->getMaterial(m_planeMaterialHandle);
        if (planeMaterial) {
            planeMaterial->setBaseColor({0.5f, 0.5f, 0.5f, 1.0f}); // gray
            planeMaterial->setMetallic(0.0f);
            planeMaterial->setRoughness(0.8f);
            planeMaterial->setReflectance(0.3f);
        }

        auto planeEntity = world.createEntity("Ground");
        auto& planeRenderer = world.addComponent<fe::MeshRendererComponent>(planeEntity);
        planeRenderer.mesh = m_planeMeshHandle;
        planeRenderer.material = m_planeMaterialHandle;
        planeRenderer.castShadows = false;
        planeRenderer.receiveShadows = true;

        // Ground plane transform
        auto& planeTransform = world.getComponent<fe::TransformComponent>(planeEntity);
        planeTransform.position = {0, -0.55f, 0};
        planeTransform.scale = {1.0f, 0.02f, 1.0f};
        planeTransform.dirty = true;
        auto lightEntity = world.createEntity("Sun");
        auto& light = world.addComponent<fe::LightComponent>(lightEntity);
        light.type = fe::LightComponent::Type::Directional;
        light.color = {1.0f, 1.0f, 0.95f};
        light.intensity = 100000.0f;
        light.castShadows = true;
        auto& lightTransform = world.getComponent<fe::TransformComponent>(lightEntity);
        lightTransform.position = {0, 5, 5};
        float angle = -0.785f; // ~45 degrees
        lightTransform.rotation = fe::Quat{std::cos(angle / 2.0f), std::sin(angle / 2.0f), 0, 0};
        auto cameraEntity = world.createEntity("Camera");
        auto& cam = world.addComponent<fe::CameraComponent>(cameraEntity);
        cam.isActive = true;
        cam.fov = 60.0f;
        cam.nearPlane = 0.1f;
        cam.farPlane = 100.0f;

        auto& camTransform = world.getComponent<fe::TransformComponent>(cameraEntity);
        camTransform.position = {0, 2, 5};

        FE_LOG_INFO("Hello Cube initialized! Controls: WASD move, Right-click+drag to look, Scroll for speed, Shift for fast");
    }

    void onUpdate(float dt) override {
        m_rotation += dt * 1.5f;

        auto& world = getWorld();
        auto& transform = world.getComponent<fe::TransformComponent>(m_cubeEntity);

        // Rotate around Y axis
        float halfAngle = m_rotation * 0.5f;
        transform.rotation = fe::Quat{std::cos(halfAngle), 0, std::sin(halfAngle), 0};
        transform.dirty = true;
    }

    void onShutdown() override {
        FE_LOG_INFO("Hello Cube shutting down");
    }

private:
    static fe::ApplicationConfig makeConfig() {
        fe::ApplicationConfig config;
        config.window.title = "Filament Engine - Hello Cube";
        config.window.width = 1280;
        config.window.height = 720;
        config.backend = fe::GraphicsBackend::Default;
        return config;
    }

    entt::entity m_cubeEntity{entt::null};
    fe::ResourceHandle<fe::Mesh> m_cubeMeshHandle;
    fe::ResourceHandle<fe::MaterialWrapper> m_materialHandle;
    fe::ResourceHandle<fe::Mesh> m_planeMeshHandle;
    fe::ResourceHandle<fe::MaterialWrapper> m_planeMaterialHandle;
    float m_rotation = 0.0f;
};

int main(int argc, char** argv) {
    HelloCubeApp app;
    app.run();
    return 0;
}
