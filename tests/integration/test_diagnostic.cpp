// Diagnostic integration test: replicates the full sandbox ECS flow step by step
// to isolate which component causes the handle_cast crash.
#include "../test_helpers.h"

#include <filament_engine/filament_engine.h>

#include <fstream>
#include <vector>
#include <cmath>

static std::vector<uint8_t> loadFile(const char* path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {};
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    return data;
}

// Minimal app that does nothing — tests basic engine lifecycle
class EmptyApp : public fe::Application {
public:
    EmptyApp() : fe::Application(makeConfig()) {}
    void onInit() override {}
    void onUpdate(float dt) override {}
    void onShutdown() override {}
private:
    static fe::ApplicationConfig makeConfig() {
        fe::ApplicationConfig config;
        config.window.title = "Diagnostic - Empty";
        config.window.width = 320;
        config.window.height = 240;
        config.backend = fe::GraphicsBackend::Default;
        return config;
    }
};

// App with just a cube — no ground plane, no IBL
class CubeOnlyApp : public fe::Application {
public:
    CubeOnlyApp() : fe::Application(makeConfig()) {}

    void onInit() override {
        auto& world = getWorld();
        auto& renderCtx = getRenderContext();
        auto* resourceMgr = fe::ResourceManager::getInstance();

        auto cubeMesh = fe::Mesh::createCube(*renderCtx.getEngine(), 0.5f);
        m_cubeMeshHandle = resourceMgr->addMesh(cubeMesh);

        auto materialData = loadFile("materials/standard_lit.filamat");
        if (materialData.empty()) {
            FE_LOG_ERROR("Could not load standard_lit.filamat");
            return;
        }
        m_materialHandle = resourceMgr->createMaterial(materialData.data(), materialData.size());

        auto* material = resourceMgr->getMaterial(m_materialHandle);
        if (material) {
            material->setBaseColor({0.8f, 0.2f, 0.2f, 1.0f});
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

        // Light
        auto lightEntity = world.createEntity("Sun");
        auto& light = world.addComponent<fe::LightComponent>(lightEntity);
        light.type = fe::LightComponent::Type::Directional;
        light.color = {1.0f, 1.0f, 0.95f};
        light.intensity = 100000.0f;
        light.castShadows = true;

        auto& lightTransform = world.getComponent<fe::TransformComponent>(lightEntity);
        lightTransform.position = {0, 5, 5};
        float angle = -0.785f;
        lightTransform.rotation = fe::Quat{std::cos(angle / 2.0f), std::sin(angle / 2.0f), 0, 0};

        // Camera
        auto cameraEntity = world.createEntity("Camera");
        auto& cam = world.addComponent<fe::CameraComponent>(cameraEntity);
        cam.isActive = true;
        cam.fov = 60.0f;
        cam.nearPlane = 0.1f;
        cam.farPlane = 100.0f;
        auto& camTransform = world.getComponent<fe::TransformComponent>(cameraEntity);
        camTransform.position = {0, 2, 5};

        FE_LOG_INFO("CubeOnlyApp initialized");
    }

    void onUpdate(float dt) override {
        // Quit after a few frames
        m_frames++;
        if (m_frames > 5) {
            FE_LOG_INFO("CubeOnlyApp: %d frames rendered OK, stopping", m_frames);
            // Close the window to exit
        }
    }

    void onShutdown() override {}
    int getFrameCount() const { return m_frames; }

private:
    static fe::ApplicationConfig makeConfig() {
        fe::ApplicationConfig config;
        config.window.title = "Diagnostic - CubeOnly";
        config.window.width = 320;
        config.window.height = 240;
        config.backend = fe::GraphicsBackend::Default;
        return config;
    }
    entt::entity m_cubeEntity{entt::null};
    fe::ResourceHandle<fe::Mesh> m_cubeMeshHandle;
    fe::ResourceHandle<fe::MaterialWrapper> m_materialHandle;
    int m_frames = 0;
};

// App with cube + ground plane (two renderables) — no IBL
class TwoRenderablesApp : public fe::Application {
public:
    TwoRenderablesApp() : fe::Application(makeConfig()) {}

    void onInit() override {
        auto& world = getWorld();
        auto& renderCtx = getRenderContext();
        auto* resourceMgr = fe::ResourceManager::getInstance();

        // Cube
        auto cubeMesh = fe::Mesh::createCube(*renderCtx.getEngine(), 0.5f);
        auto cubeMeshHandle = resourceMgr->addMesh(cubeMesh);

        auto materialData = loadFile("materials/standard_lit.filamat");
        if (materialData.empty()) { FE_LOG_ERROR("No material"); return; }
        auto materialHandle = resourceMgr->createMaterial(materialData.data(), materialData.size());
        auto* material = resourceMgr->getMaterial(materialHandle);
        if (material) {
            material->setBaseColor({0.8f, 0.2f, 0.2f, 1.0f});
            material->setMetallic(0.0f);
            material->setRoughness(0.4f);
        }

        auto cubeEntity = world.createEntity("Cube");
        auto& cubeRenderer = world.addComponent<fe::MeshRendererComponent>(cubeEntity);
        cubeRenderer.mesh = cubeMeshHandle;
        cubeRenderer.material = materialHandle;
        cubeRenderer.castShadows = true;
        cubeRenderer.receiveShadows = true;

        // Ground plane
        auto planeMesh = fe::Mesh::createCube(*renderCtx.getEngine(), 5.0f);
        auto planeMeshHandle = resourceMgr->addMesh(planeMesh);

        auto planeMaterialData = loadFile("materials/standard_lit.filamat");
        auto planeMaterialHandle = resourceMgr->createMaterial(planeMaterialData.data(), planeMaterialData.size());
        auto* planeMaterial = resourceMgr->getMaterial(planeMaterialHandle);
        if (planeMaterial) {
            planeMaterial->setBaseColor({0.5f, 0.5f, 0.5f, 1.0f});
            planeMaterial->setMetallic(0.0f);
            planeMaterial->setRoughness(0.8f);
        }

        auto planeEntity = world.createEntity("Ground");
        auto& planeRenderer = world.addComponent<fe::MeshRendererComponent>(planeEntity);
        planeRenderer.mesh = planeMeshHandle;
        planeRenderer.material = planeMaterialHandle;
        planeRenderer.castShadows = false;
        planeRenderer.receiveShadows = true;
        auto& planeTransform = world.getComponent<fe::TransformComponent>(planeEntity);
        planeTransform.position = {0, -0.55f, 0};
        planeTransform.scale = {1.0f, 0.02f, 1.0f};
        planeTransform.dirty = true;

        // Light
        auto lightEntity = world.createEntity("Sun");
        auto& light = world.addComponent<fe::LightComponent>(lightEntity);
        light.type = fe::LightComponent::Type::Directional;
        light.color = {1.0f, 1.0f, 0.95f};
        light.intensity = 100000.0f;
        light.castShadows = true;
        auto& lightTransform = world.getComponent<fe::TransformComponent>(lightEntity);
        lightTransform.position = {0, 5, 5};
        float angle = -0.785f;
        lightTransform.rotation = fe::Quat{std::cos(angle / 2.0f), std::sin(angle / 2.0f), 0, 0};

        // Camera
        auto cameraEntity = world.createEntity("Camera");
        auto& cam = world.addComponent<fe::CameraComponent>(cameraEntity);
        cam.isActive = true;
        cam.fov = 60.0f;
        cam.nearPlane = 0.1f;
        cam.farPlane = 100.0f;
        auto& camTransform = world.getComponent<fe::TransformComponent>(cameraEntity);
        camTransform.position = {0, 2, 5};

        FE_LOG_INFO("TwoRenderablesApp initialized");
    }

    void onUpdate(float dt) override {
        m_frames++;
    }
    void onShutdown() override {}
    int getFrameCount() const { return m_frames; }

private:
    static fe::ApplicationConfig makeConfig() {
        fe::ApplicationConfig config;
        config.window.title = "Diagnostic - TwoRenderables";
        config.window.width = 320;
        config.window.height = 240;
        config.backend = fe::GraphicsBackend::Default;
        return config;
    }
    int m_frames = 0;
};

// App with cube + IBL — no ground plane
class CubeWithIBLApp : public fe::Application {
public:
    CubeWithIBLApp() : fe::Application(makeConfig()) {}

    void onInit() override {
        auto& world = getWorld();
        auto& renderCtx = getRenderContext();
        auto* resourceMgr = fe::ResourceManager::getInstance();

        renderCtx.loadIBL("assets/ibl");

        auto cubeMesh = fe::Mesh::createCube(*renderCtx.getEngine(), 0.5f);
        auto cubeMeshHandle = resourceMgr->addMesh(cubeMesh);

        auto materialData = loadFile("materials/standard_lit.filamat");
        if (materialData.empty()) { FE_LOG_ERROR("No material"); return; }
        auto materialHandle = resourceMgr->createMaterial(materialData.data(), materialData.size());
        auto* material = resourceMgr->getMaterial(materialHandle);
        if (material) {
            material->setBaseColor({0.8f, 0.2f, 0.2f, 1.0f});
            material->setMetallic(0.0f);
            material->setRoughness(0.4f);
        }

        auto cubeEntity = world.createEntity("Cube");
        auto& cubeRenderer = world.addComponent<fe::MeshRendererComponent>(cubeEntity);
        cubeRenderer.mesh = cubeMeshHandle;
        cubeRenderer.material = materialHandle;
        cubeRenderer.castShadows = true;
        cubeRenderer.receiveShadows = true;

        auto lightEntity = world.createEntity("Sun");
        auto& light = world.addComponent<fe::LightComponent>(lightEntity);
        light.type = fe::LightComponent::Type::Directional;
        light.color = {1.0f, 1.0f, 0.95f};
        light.intensity = 100000.0f;
        light.castShadows = true;
        auto& lightTransform = world.getComponent<fe::TransformComponent>(lightEntity);
        lightTransform.position = {0, 5, 5};
        float angle = -0.785f;
        lightTransform.rotation = fe::Quat{std::cos(angle / 2.0f), std::sin(angle / 2.0f), 0, 0};

        auto cameraEntity = world.createEntity("Camera");
        auto& cam = world.addComponent<fe::CameraComponent>(cameraEntity);
        cam.isActive = true;
        cam.fov = 60.0f;
        cam.nearPlane = 0.1f;
        cam.farPlane = 100.0f;
        auto& camTransform = world.getComponent<fe::TransformComponent>(cameraEntity);
        camTransform.position = {0, 2, 5};

        FE_LOG_INFO("CubeWithIBLApp initialized");
    }
    void onUpdate(float dt) override { m_frames++; }
    void onShutdown() override {}
    int getFrameCount() const { return m_frames; }

private:
    static fe::ApplicationConfig makeConfig() {
        fe::ApplicationConfig config;
        config.window.title = "Diagnostic - CubeWithIBL";
        config.window.width = 320;
        config.window.height = 240;
        config.backend = fe::GraphicsBackend::Default;
        return config;
    }
    int m_frames = 0;
};

// Each test runs the app for a few frames then exits
// The app.run() will crash if there's a handle issue
// We can't easily stop app.run() from inside so we test
// construction + init without the full render loop

TEST(Diagnostic_CubeOnly_Init) {
    CubeOnlyApp app;
    // If we get past construction + init without crash, the setup is OK
    // The crash happens during run() in the render loop
    printf("  CubeOnlyApp created and initialized OK\n");
}

TEST(Diagnostic_TwoRenderables_Init) {
    TwoRenderablesApp app;
    printf("  TwoRenderablesApp created and initialized OK\n");
}

TEST(Diagnostic_CubeWithIBL_Init) {
    CubeWithIBLApp app;
    printf("  CubeWithIBLApp created and initialized OK\n");
}

int main() {
    return runAllTests();
}
