// Unit tests for ECS Components
// NOTE: EnTT headers must be included BEFORE GTest to avoid
// entt_traits<long long> template instantiation conflict.
#include <filament_engine/ecs/components.h>
#include <gtest/gtest.h>

// TransformComponent

TEST(TransformComponent, DefaultValues) {
    fe::TransformComponent t;
    EXPECT_FLOAT_EQ(t.position.x, 0.0f);
    EXPECT_FLOAT_EQ(t.position.y, 0.0f);
    EXPECT_FLOAT_EQ(t.position.z, 0.0f);
    EXPECT_FLOAT_EQ(t.scale.x, 1.0f);
    EXPECT_FLOAT_EQ(t.scale.y, 1.0f);
    EXPECT_FLOAT_EQ(t.scale.z, 1.0f);
    EXPECT_TRUE(t.dirty);
    EXPECT_TRUE(t.parent == entt::null);
}

TEST(TransformComponent, IdentityRotation) {
    fe::TransformComponent t;
    // Identity quaternion: w=1, x=0, y=0, z=0
    EXPECT_FLOAT_EQ(t.rotation.w, 1.0f);
    EXPECT_FLOAT_EQ(t.rotation.x, 0.0f);
    EXPECT_FLOAT_EQ(t.rotation.y, 0.0f);
    EXPECT_FLOAT_EQ(t.rotation.z, 0.0f);
}

TEST(TransformComponent, ModifyPosition) {
    fe::TransformComponent t;
    t.position = {1.0f, 2.0f, 3.0f};
    EXPECT_FLOAT_EQ(t.position.x, 1.0f);
    EXPECT_FLOAT_EQ(t.position.y, 2.0f);
    EXPECT_FLOAT_EQ(t.position.z, 3.0f);
}

TEST(TransformComponent, ModifyScale) {
    fe::TransformComponent t;
    t.scale = {2.0f, 3.0f, 4.0f};
    EXPECT_FLOAT_EQ(t.scale.x, 2.0f);
    EXPECT_FLOAT_EQ(t.scale.y, 3.0f);
    EXPECT_FLOAT_EQ(t.scale.z, 4.0f);
}

TEST(TransformComponent, DirtyFlag_InitiallyTrue) {
    fe::TransformComponent t;
    EXPECT_TRUE(t.dirty);
}

TEST(TransformComponent, DirtyFlag_CanBeClearedAndSet) {
    fe::TransformComponent t;
    t.dirty = false;
    EXPECT_FALSE(t.dirty);
    t.dirty = true;
    EXPECT_TRUE(t.dirty);
}

TEST(TransformComponent, Parent_DefaultNull) {
    fe::TransformComponent t;
    EXPECT_TRUE(t.parent == entt::null);
}

TEST(TransformComponent, Parent_CanBeAssigned) {
    entt::registry reg;
    auto parentEntity = reg.create();

    fe::TransformComponent t;
    t.parent = parentEntity;
    EXPECT_TRUE(t.parent != entt::null);
    EXPECT_TRUE(t.parent == parentEntity);
}

// TagComponent

TEST(TagComponent, CanSetName) {
    fe::TagComponent tag;
    tag.name = "TestEntity";
    EXPECT_EQ(tag.name, "TestEntity");
}

TEST(TagComponent, EmptyByDefault) {
    fe::TagComponent tag;
    EXPECT_TRUE(tag.name.empty());
}

// CameraComponent

TEST(CameraComponent, DefaultValues) {
    fe::CameraComponent cam;
    EXPECT_FLOAT_EQ(cam.fov, 60.0f);
    EXPECT_FLOAT_EQ(cam.nearPlane, 0.1f);
    EXPECT_FLOAT_EQ(cam.farPlane, 1000.0f);
    EXPECT_FALSE(cam.isActive);
    EXPECT_TRUE(cam.dirty);
}

TEST(CameraComponent, SetActive) {
    fe::CameraComponent cam;
    cam.isActive = true;
    EXPECT_TRUE(cam.isActive);
}

TEST(CameraComponent, ModifyFOV) {
    fe::CameraComponent cam;
    cam.fov = 90.0f;
    EXPECT_FLOAT_EQ(cam.fov, 90.0f);
}

TEST(CameraComponent, ModifyClipPlanes) {
    fe::CameraComponent cam;
    cam.nearPlane = 0.5f;
    cam.farPlane = 500.0f;
    EXPECT_FLOAT_EQ(cam.nearPlane, 0.5f);
    EXPECT_FLOAT_EQ(cam.farPlane, 500.0f);
}

// LightComponent

TEST(LightComponent, DefaultValues) {
    fe::LightComponent light;
    EXPECT_EQ(light.type, fe::LightComponent::Type::Point);
    EXPECT_FLOAT_EQ(light.intensity, 100000.0f);
    EXPECT_FALSE(light.castShadows);
    EXPECT_FALSE(light.initialized);
}

TEST(LightComponent, SetType_Directional) {
    fe::LightComponent light;
    light.type = fe::LightComponent::Type::Directional;
    EXPECT_EQ(light.type, fe::LightComponent::Type::Directional);
}

TEST(LightComponent, SetType_Spot) {
    fe::LightComponent light;
    light.type = fe::LightComponent::Type::Spot;
    EXPECT_EQ(light.type, fe::LightComponent::Type::Spot);
}

TEST(LightComponent, SpotAngles_Default) {
    fe::LightComponent light;
    EXPECT_FLOAT_EQ(light.innerConeAngle, 0.0f);
    EXPECT_FLOAT_EQ(light.outerConeAngle, 0.5f);
}

TEST(LightComponent, SpotAngles_CanModify) {
    fe::LightComponent light;
    light.innerConeAngle = 0.3f;
    light.outerConeAngle = 0.8f;
    EXPECT_FLOAT_EQ(light.innerConeAngle, 0.3f);
    EXPECT_FLOAT_EQ(light.outerConeAngle, 0.8f);
}

TEST(LightComponent, SetColor) {
    fe::LightComponent light;
    light.color = {0.5f, 0.7f, 0.9f};
    EXPECT_FLOAT_EQ(light.color.x, 0.5f);
    EXPECT_FLOAT_EQ(light.color.y, 0.7f);
    EXPECT_FLOAT_EQ(light.color.z, 0.9f);
}

TEST(LightComponent, SetRadius) {
    fe::LightComponent light;
    EXPECT_FLOAT_EQ(light.radius, 10.0f);
    light.radius = 25.0f;
    EXPECT_FLOAT_EQ(light.radius, 25.0f);
}

TEST(LightComponent, CastShadows) {
    fe::LightComponent light;
    light.castShadows = true;
    EXPECT_TRUE(light.castShadows);
}

// MeshRendererComponent

TEST(MeshRendererComponent, DefaultValues) {
    fe::MeshRendererComponent mr;
    EXPECT_FALSE(mr.mesh.isValid());
    EXPECT_FALSE(mr.material.isValid());
    EXPECT_TRUE(mr.castShadows);
    EXPECT_TRUE(mr.receiveShadows);
    EXPECT_FALSE(mr.initialized);
}

TEST(MeshRendererComponent, ToggleShadows) {
    fe::MeshRendererComponent mr;
    mr.castShadows = false;
    mr.receiveShadows = false;
    EXPECT_FALSE(mr.castShadows);
    EXPECT_FALSE(mr.receiveShadows);
}

TEST(MeshRendererComponent, SetValidHandles) {
    fe::MeshRendererComponent mr;
    mr.mesh = fe::ResourceHandle<fe::Mesh>(1);
    mr.material = fe::ResourceHandle<fe::MaterialWrapper>(2);
    EXPECT_TRUE(mr.mesh.isValid());
    EXPECT_TRUE(mr.material.isValid());
    EXPECT_EQ(mr.mesh.getId(), 1u);
    EXPECT_EQ(mr.material.getId(), 2u);
}
