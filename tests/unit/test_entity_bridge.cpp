// Unit tests for EntityBridge (EnTT <-> Filament entity mapping)
#include <gtest/gtest.h>
#include <filament_engine/ecs/entity_bridge.h>

// =====================
// Link
// =====================

TEST(EntityBridge, Link_CreatesFilamentEntity) {
    entt::registry registry;
    fe::EntityBridge bridge;
    auto enttEntity = registry.create();

    auto filamentEntity = bridge.link(registry, enttEntity);
    // The Filament entity should have been created (non-zero id)
    EXPECT_NE(filamentEntity.getId(), 0u);
}

TEST(EntityBridge, Link_AddsFilamentEntityComponent) {
    entt::registry registry;
    fe::EntityBridge bridge;
    auto enttEntity = registry.create();

    bridge.link(registry, enttEntity);
    EXPECT_TRUE(registry.all_of<fe::FilamentEntityComponent>(enttEntity));
}

TEST(EntityBridge, Link_ComponentHoldsCorrectEntity) {
    entt::registry registry;
    fe::EntityBridge bridge;
    auto enttEntity = registry.create();

    auto filamentEntity = bridge.link(registry, enttEntity);
    auto& comp = registry.get<fe::FilamentEntityComponent>(enttEntity);
    EXPECT_EQ(comp.filamentEntity.getId(), filamentEntity.getId());
}

// =====================
// HasFilamentEntity
// =====================

TEST(EntityBridge, HasFilamentEntity_FalseBeforeLink) {
    entt::registry registry;
    fe::EntityBridge bridge;
    auto enttEntity = registry.create();

    EXPECT_FALSE(bridge.hasFilamentEntity(registry, enttEntity));
}

TEST(EntityBridge, HasFilamentEntity_TrueAfterLink) {
    entt::registry registry;
    fe::EntityBridge bridge;
    auto enttEntity = registry.create();

    bridge.link(registry, enttEntity);
    EXPECT_TRUE(bridge.hasFilamentEntity(registry, enttEntity));
}

// =====================
// Lookups
// =====================

TEST(EntityBridge, GetFilamentEntity_ReturnsCorrect) {
    entt::registry registry;
    fe::EntityBridge bridge;
    auto enttEntity = registry.create();

    auto filamentEntity = bridge.link(registry, enttEntity);
    auto result = bridge.getFilamentEntity(registry, enttEntity);
    EXPECT_EQ(result.getId(), filamentEntity.getId());
}

TEST(EntityBridge, GetEnttEntity_ReturnsCorrect) {
    entt::registry registry;
    fe::EntityBridge bridge;
    auto enttEntity = registry.create();

    auto filamentEntity = bridge.link(registry, enttEntity);
    auto result = bridge.getEnttEntity(filamentEntity);
    EXPECT_EQ(result, enttEntity);
}

TEST(EntityBridge, GetEnttEntity_UnknownFilamentEntity_ReturnsNull) {
    fe::EntityBridge bridge;
    utils::Entity unknownEntity;

    auto result = bridge.getEnttEntity(unknownEntity);
    EXPECT_TRUE(result == entt::null);
}

TEST(EntityBridge, GetFilamentEntity_UnlinkedEntity_ReturnsDefault) {
    entt::registry registry;
    fe::EntityBridge bridge;
    auto enttEntity = registry.create();

    auto result = bridge.getFilamentEntity(registry, enttEntity);
    // Default-constructed utils::Entity should have id 0
    EXPECT_EQ(result.getId(), 0u);
}

// =====================
// Unlink
// =====================

TEST(EntityBridge, Unlink_RemovesComponent) {
    entt::registry registry;
    fe::EntityBridge bridge;
    auto enttEntity = registry.create();

    bridge.link(registry, enttEntity);
    EXPECT_TRUE(bridge.hasFilamentEntity(registry, enttEntity));

    bridge.unlink(registry, enttEntity);
    EXPECT_FALSE(bridge.hasFilamentEntity(registry, enttEntity));
}

TEST(EntityBridge, Unlink_RemovesReverseLookup) {
    entt::registry registry;
    fe::EntityBridge bridge;
    auto enttEntity = registry.create();

    auto filamentEntity = bridge.link(registry, enttEntity);
    bridge.unlink(registry, enttEntity);

    auto result = bridge.getEnttEntity(filamentEntity);
    EXPECT_TRUE(result == entt::null);
}

TEST(EntityBridge, Unlink_NonLinkedEntity_NoOp) {
    entt::registry registry;
    fe::EntityBridge bridge;
    auto enttEntity = registry.create();

    // Should not crash
    EXPECT_NO_THROW(bridge.unlink(registry, enttEntity));
}

// =====================
// Multiple entities
// =====================

TEST(EntityBridge, MultipleEntities_IndependentLinks) {
    entt::registry registry;
    fe::EntityBridge bridge;

    auto entity1 = registry.create();
    auto entity2 = registry.create();
    auto entity3 = registry.create();

    auto filament1 = bridge.link(registry, entity1);
    auto filament2 = bridge.link(registry, entity2);
    auto filament3 = bridge.link(registry, entity3);

    // All should have different Filament entities
    EXPECT_NE(filament1.getId(), filament2.getId());
    EXPECT_NE(filament2.getId(), filament3.getId());
    EXPECT_NE(filament1.getId(), filament3.getId());

    // Reverse lookups should work independently
    EXPECT_EQ(bridge.getEnttEntity(filament1), entity1);
    EXPECT_EQ(bridge.getEnttEntity(filament2), entity2);
    EXPECT_EQ(bridge.getEnttEntity(filament3), entity3);
}

TEST(EntityBridge, MultipleEntities_UnlinkOne_OthersUnaffected) {
    entt::registry registry;
    fe::EntityBridge bridge;

    auto entity1 = registry.create();
    auto entity2 = registry.create();

    auto filament1 = bridge.link(registry, entity1);
    auto filament2 = bridge.link(registry, entity2);

    bridge.unlink(registry, entity1);

    // entity1 should be unlinked
    EXPECT_FALSE(bridge.hasFilamentEntity(registry, entity1));
    EXPECT_TRUE(bridge.getEnttEntity(filament1) == entt::null);

    // entity2 should still be linked
    EXPECT_TRUE(bridge.hasFilamentEntity(registry, entity2));
    EXPECT_EQ(bridge.getEnttEntity(filament2), entity2);
}
