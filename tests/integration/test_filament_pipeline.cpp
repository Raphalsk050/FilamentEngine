// Integration test: minimal Filament pipeline
// Tests engine creation, resource building, and rendering without a window.
#include <gtest/gtest.h>

#include <filament/Engine.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/View.h>
#include <filament/Camera.h>
#include <filament/Skybox.h>
#include <filament/IndirectLight.h>
#include <filament/LightManager.h>
#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>
#include <filament/Texture.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>

#include <filament_engine/resources/mesh.h>
#include <filament_engine/resources/material.h>
#include <filament_engine/core/log.h>

#include <utils/EntityManager.h>
#include <utils/Entity.h>

#include <fstream>
#include <vector>

// Loads a binary file
static std::vector<uint8_t> loadFile(const char* path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {};
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    return data;
}

TEST(FilamentPipeline, CreateAndDestroy) {
    auto* engine = filament::Engine::create(filament::Engine::Backend::METAL);
    ASSERT_NE(engine, nullptr);

    auto* renderer = engine->createRenderer();
    ASSERT_NE(renderer, nullptr);

    auto* scene = engine->createScene();
    ASSERT_NE(scene, nullptr);

    auto* view = engine->createView();
    ASSERT_NE(view, nullptr);

    auto cameraEntity = utils::EntityManager::get().create();
    auto* camera = engine->createCamera(cameraEntity);
    ASSERT_NE(camera, nullptr);

    view->setScene(scene);
    view->setCamera(camera);

    // Cleanup (reverse order)
    engine->destroyCameraComponent(cameraEntity);
    utils::EntityManager::get().destroy(cameraEntity);
    engine->destroy(view);
    engine->destroy(scene);
    engine->destroy(renderer);
    filament::Engine::destroy(&engine);
}

TEST(FilamentPipeline, CreateSkybox) {
    auto* engine = filament::Engine::create(filament::Engine::Backend::METAL);
    ASSERT_NE(engine, nullptr);

    auto* scene = engine->createScene();

    auto* skybox = filament::Skybox::Builder()
        .color({0.1f, 0.1f, 0.2f, 1.0f})
        .build(*engine);
    ASSERT_NE(skybox, nullptr);
    scene->setSkybox(skybox);

    engine->destroy(skybox);
    engine->destroy(scene);
    filament::Engine::destroy(&engine);
}

TEST(FilamentPipeline, CreateDirectionalLight) {
    auto* engine = filament::Engine::create(filament::Engine::Backend::METAL);
    ASSERT_NE(engine, nullptr);

    auto* scene = engine->createScene();

    auto lightEntity = utils::EntityManager::get().create();
    filament::LightManager::Builder(filament::LightManager::Type::DIRECTIONAL)
        .color({1.0f, 1.0f, 0.95f})
        .intensity(100000.0f)
        .direction({0, -1, -1})
        .castShadows(true)
        .build(*engine, lightEntity);

    scene->addEntity(lightEntity);

    auto& lightMgr = engine->getLightManager();
    auto instance = lightMgr.getInstance(lightEntity);
    EXPECT_TRUE(instance.isValid());

    lightMgr.setDirection(instance, {0, -1, 0});
    lightMgr.setColor(instance, {1.0f, 0.8f, 0.6f});
    lightMgr.setIntensity(instance, 50000.0f);

    scene->remove(lightEntity);
    engine->destroy(lightEntity);
    utils::EntityManager::get().destroy(lightEntity);
    engine->destroy(scene);
    filament::Engine::destroy(&engine);
}

TEST(FilamentPipeline, CreateCubeRenderable) {
    auto* engine = filament::Engine::create(filament::Engine::Backend::METAL);
    ASSERT_NE(engine, nullptr);

    auto* scene = engine->createScene();

    auto cube = fe::Mesh::createCube(*engine, 0.5f);
    ASSERT_NE(cube.vertexBuffer, nullptr);
    ASSERT_NE(cube.indexBuffer, nullptr);
    EXPECT_GT(cube.indexCount, 0u);

    auto materialData = loadFile("materials/standard_lit.filamat");
    if (materialData.empty()) {
        GTEST_SKIP() << "standard_lit.filamat not found (run from build/sandbox dir)";
    }

    auto* material = filament::Material::Builder()
        .package(materialData.data(), materialData.size())
        .build(*engine);
    ASSERT_NE(material, nullptr);

    auto* instance = material->createInstance();
    ASSERT_NE(instance, nullptr);

    auto entity = utils::EntityManager::get().create();
    filament::RenderableManager::Builder(1)
        .boundingBox(cube.boundingBox)
        .material(0, instance)
        .geometry(0, filament::RenderableManager::PrimitiveType::TRIANGLES,
                  cube.vertexBuffer, cube.indexBuffer, 0, cube.indexCount)
        .culling(false)
        .receiveShadows(true)
        .castShadows(true)
        .build(*engine, entity);

    scene->addEntity(entity);

    scene->remove(entity);
    engine->destroy(entity);
    utils::EntityManager::get().destroy(entity);
    engine->destroy(instance);
    engine->destroy(material);
    engine->destroy(cube.vertexBuffer);
    engine->destroy(cube.indexBuffer);
    engine->destroy(scene);
    filament::Engine::destroy(&engine);
}

TEST(FilamentPipeline, TwoRenderables_SameMaterial) {
    auto* engine = filament::Engine::create(filament::Engine::Backend::METAL);
    ASSERT_NE(engine, nullptr);

    auto* scene = engine->createScene();

    auto materialData = loadFile("materials/standard_lit.filamat");
    if (materialData.empty()) {
        engine->destroy(scene);
        filament::Engine::destroy(&engine);
        GTEST_SKIP() << "standard_lit.filamat not found";
    }

    auto* material1 = filament::Material::Builder()
        .package(materialData.data(), materialData.size())
        .build(*engine);
    ASSERT_NE(material1, nullptr);

    auto* material2 = filament::Material::Builder()
        .package(materialData.data(), materialData.size())
        .build(*engine);
    ASSERT_NE(material2, nullptr);

    auto* instance1 = material1->createInstance();
    auto* instance2 = material2->createInstance();
    ASSERT_NE(instance1, nullptr);
    ASSERT_NE(instance2, nullptr);

    auto cube1 = fe::Mesh::createCube(*engine, 0.5f);
    auto cube2 = fe::Mesh::createCube(*engine, 5.0f);

    auto entity1 = utils::EntityManager::get().create();
    auto entity2 = utils::EntityManager::get().create();

    filament::RenderableManager::Builder(1)
        .boundingBox(cube1.boundingBox)
        .material(0, instance1)
        .geometry(0, filament::RenderableManager::PrimitiveType::TRIANGLES,
                  cube1.vertexBuffer, cube1.indexBuffer, 0, cube1.indexCount)
        .castShadows(true).receiveShadows(true)
        .build(*engine, entity1);

    filament::RenderableManager::Builder(1)
        .boundingBox(cube2.boundingBox)
        .material(0, instance2)
        .geometry(0, filament::RenderableManager::PrimitiveType::TRIANGLES,
                  cube2.vertexBuffer, cube2.indexBuffer, 0, cube2.indexCount)
        .castShadows(false).receiveShadows(true)
        .build(*engine, entity2);

    scene->addEntity(entity1);
    scene->addEntity(entity2);

    auto cameraEntity = utils::EntityManager::get().create();
    auto* camera = engine->createCamera(cameraEntity);
    auto* view = engine->createView();
    view->setScene(scene);
    view->setCamera(camera);

    auto* renderer = engine->createRenderer();

    // Cleanup
    engine->destroyCameraComponent(cameraEntity);
    utils::EntityManager::get().destroy(cameraEntity);
    engine->destroy(renderer);
    engine->destroy(view);

    scene->remove(entity1);
    scene->remove(entity2);
    engine->destroy(entity1);
    engine->destroy(entity2);
    utils::EntityManager::get().destroy(entity1);
    utils::EntityManager::get().destroy(entity2);

    engine->destroy(instance1);
    engine->destroy(instance2);
    engine->destroy(material1);
    engine->destroy(material2);
    engine->destroy(cube1.vertexBuffer);
    engine->destroy(cube1.indexBuffer);
    engine->destroy(cube2.vertexBuffer);
    engine->destroy(cube2.indexBuffer);
    engine->destroy(scene);
    filament::Engine::destroy(&engine);
}
