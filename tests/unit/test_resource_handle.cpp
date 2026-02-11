// Unit tests for ResourceHandle<T>
#include "../test_helpers.h"
#include <filament_engine/resources/resource_handle.h>

// Dummy types for testing
struct DummyMesh {};
struct DummyMaterial {};

TEST(ResourceHandle_DefaultConstructor_IsInvalid) {
    fe::ResourceHandle<DummyMesh> handle;
    ASSERT_FALSE(handle.isValid());
    ASSERT_EQ(handle.getId(), fe::ResourceHandle<DummyMesh>::INVALID_ID);
}

TEST(ResourceHandle_WithId_IsValid) {
    fe::ResourceHandle<DummyMesh> handle(1);
    ASSERT_TRUE(handle.isValid());
    ASSERT_EQ(handle.getId(), 1u);
}

TEST(ResourceHandle_Equality) {
    fe::ResourceHandle<DummyMesh> a(5);
    fe::ResourceHandle<DummyMesh> b(5);
    fe::ResourceHandle<DummyMesh> c(10);

    ASSERT_TRUE(a == b);
    ASSERT_TRUE(a != c);
    ASSERT_FALSE(a == c);
}

TEST(ResourceHandle_BoolConversion) {
    fe::ResourceHandle<DummyMesh> valid(1);
    fe::ResourceHandle<DummyMesh> invalid;

    ASSERT_TRUE(static_cast<bool>(valid));
    ASSERT_FALSE(static_cast<bool>(invalid));
}

TEST(ResourceHandle_DifferentTypes_AreIndependent) {
    fe::ResourceHandle<DummyMesh> mesh(1);
    fe::ResourceHandle<DummyMaterial> material(1);

    // Both have ID 1 but are different types - should compile and work independently
    ASSERT_EQ(mesh.getId(), 1u);
    ASSERT_EQ(material.getId(), 1u);
    ASSERT_TRUE(mesh.isValid());
    ASSERT_TRUE(material.isValid());
}

TEST(ResourceHandle_InvalidId_IsZero) {
    ASSERT_EQ(fe::ResourceHandle<DummyMesh>::INVALID_ID, 0u);
}

int main() {
    return runAllTests();
}
