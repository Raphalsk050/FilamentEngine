// Unit tests for ECS Components
#include "../test_helpers.h"
#include <filament_engine/ecs/components.h>

TEST(TransformComponent_DefaultValues) {
    fe::TransformComponent t;
    ASSERT_NEAR(t.position.x, 0.0f, 1e-6f);
    ASSERT_NEAR(t.position.y, 0.0f, 1e-6f);
    ASSERT_NEAR(t.position.z, 0.0f, 1e-6f);
    ASSERT_NEAR(t.scale.x, 1.0f, 1e-6f);
    ASSERT_NEAR(t.scale.y, 1.0f, 1e-6f);
    ASSERT_NEAR(t.scale.z, 1.0f, 1e-6f);
    ASSERT_TRUE(t.dirty);
    ASSERT_EQ(t.parent, entt::null);
}

TEST(TransformComponent_ModifyPosition) {
    fe::TransformComponent t;
    t.position = {1.0f, 2.0f, 3.0f};
    ASSERT_NEAR(t.position.x, 1.0f, 1e-6f);
    ASSERT_NEAR(t.position.y, 2.0f, 1e-6f);
    ASSERT_NEAR(t.position.z, 3.0f, 1e-6f);
}

TEST(TransformComponent_ModifyScale) {
    fe::TransformComponent t;
    t.scale = {2.0f, 3.0f, 4.0f};
    ASSERT_NEAR(t.scale.x, 2.0f, 1e-6f);
    ASSERT_NEAR(t.scale.y, 3.0f, 1e-6f);
    ASSERT_NEAR(t.scale.z, 4.0f, 1e-6f);
}

TEST(TransformComponent_DirtyFlag) {
    fe::TransformComponent t;
    ASSERT_TRUE(t.dirty);
    t.dirty = false;
    ASSERT_FALSE(t.dirty);
}

TEST(CameraComponent_DefaultValues) {
    fe::CameraComponent cam;
    ASSERT_NEAR(cam.fov, 60.0f, 1e-6f);
    ASSERT_NEAR(cam.nearPlane, 0.1f, 1e-6f);
    ASSERT_NEAR(cam.farPlane, 1000.0f, 1e-6f);
    ASSERT_FALSE(cam.isActive);
    ASSERT_TRUE(cam.dirty);
}

TEST(CameraComponent_SetActive) {
    fe::CameraComponent cam;
    cam.isActive = true;
    ASSERT_TRUE(cam.isActive);
}

TEST(LightComponent_DefaultValues) {
    fe::LightComponent light;
    ASSERT_EQ(light.type, fe::LightComponent::Type::Point);
    ASSERT_NEAR(light.intensity, 100000.0f, 1e-2f);
    ASSERT_FALSE(light.castShadows);
    ASSERT_FALSE(light.initialized);
}

TEST(LightComponent_SetType) {
    fe::LightComponent light;
    light.type = fe::LightComponent::Type::Point;
    ASSERT_EQ(light.type, fe::LightComponent::Type::Point);

    light.type = fe::LightComponent::Type::Spot;
    ASSERT_EQ(light.type, fe::LightComponent::Type::Spot);
}

TEST(LightComponent_SetColor) {
    fe::LightComponent light;
    light.color = {0.5f, 0.7f, 0.9f};
    ASSERT_NEAR(light.color.x, 0.5f, 1e-6f);
    ASSERT_NEAR(light.color.y, 0.7f, 1e-6f);
    ASSERT_NEAR(light.color.z, 0.9f, 1e-6f);
}

TEST(MeshRendererComponent_DefaultValues) {
    fe::MeshRendererComponent mr;
    ASSERT_FALSE(mr.mesh.isValid());
    ASSERT_FALSE(mr.material.isValid());
    ASSERT_TRUE(mr.castShadows);
    ASSERT_TRUE(mr.receiveShadows);
    ASSERT_FALSE(mr.initialized);
}


int main() {
    return runAllTests();
}
