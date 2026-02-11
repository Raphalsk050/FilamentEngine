// Unit tests for ResourceHandle<T>
#include <gtest/gtest.h>
#include <filament_engine/resources/resource_handle.h>

#include <unordered_set>

// Dummy types for testing type safety
struct DummyMesh {};
struct DummyMaterial {};

// =====================
// Construction & Validity
// =====================

TEST(ResourceHandle, DefaultConstructor_IsInvalid) {
    fe::ResourceHandle<DummyMesh> handle;
    EXPECT_FALSE(handle.isValid());
    EXPECT_EQ(handle.getId(), fe::ResourceHandle<DummyMesh>::INVALID_ID);
}

TEST(ResourceHandle, ExplicitId_IsValid) {
    fe::ResourceHandle<DummyMesh> handle(1);
    EXPECT_TRUE(handle.isValid());
    EXPECT_EQ(handle.getId(), 1u);
}

TEST(ResourceHandle, InvalidId_IsZero) {
    EXPECT_EQ(fe::ResourceHandle<DummyMesh>::INVALID_ID, 0u);
}

TEST(ResourceHandle, ZeroId_IsInvalid) {
    fe::ResourceHandle<DummyMesh> handle(0);
    EXPECT_FALSE(handle.isValid());
}

TEST(ResourceHandle, LargeId_IsValid) {
    fe::ResourceHandle<DummyMesh> handle(UINT32_MAX);
    EXPECT_TRUE(handle.isValid());
    EXPECT_EQ(handle.getId(), UINT32_MAX);
}

// =====================
// Boolean conversion
// =====================

TEST(ResourceHandle, BoolConversion_ValidHandle) {
    fe::ResourceHandle<DummyMesh> valid(42);
    EXPECT_TRUE(static_cast<bool>(valid));
}

TEST(ResourceHandle, BoolConversion_InvalidHandle) {
    fe::ResourceHandle<DummyMesh> invalid;
    EXPECT_FALSE(static_cast<bool>(invalid));
}

// =====================
// Equality operators
// =====================

TEST(ResourceHandle, Equality_SameId) {
    fe::ResourceHandle<DummyMesh> a(5);
    fe::ResourceHandle<DummyMesh> b(5);
    EXPECT_EQ(a, b);
}

TEST(ResourceHandle, Equality_DifferentId) {
    fe::ResourceHandle<DummyMesh> a(5);
    fe::ResourceHandle<DummyMesh> c(10);
    EXPECT_NE(a, c);
}

TEST(ResourceHandle, Equality_BothInvalid) {
    fe::ResourceHandle<DummyMesh> a;
    fe::ResourceHandle<DummyMesh> b;
    EXPECT_EQ(a, b);
}

// =====================
// Type safety
// =====================

TEST(ResourceHandle, DifferentTypes_SameId_AreIndependent) {
    fe::ResourceHandle<DummyMesh> mesh(1);
    fe::ResourceHandle<DummyMaterial> material(1);

    // Both have ID 1 but are different types — compile-time type safety
    EXPECT_EQ(mesh.getId(), 1u);
    EXPECT_EQ(material.getId(), 1u);
    EXPECT_TRUE(mesh.isValid());
    EXPECT_TRUE(material.isValid());
}

// =====================
// Copy semantics
// =====================

TEST(ResourceHandle, CopyConstructor) {
    fe::ResourceHandle<DummyMesh> original(42);
    fe::ResourceHandle<DummyMesh> copy(original);
    EXPECT_EQ(copy.getId(), 42u);
    EXPECT_EQ(original, copy);
}

TEST(ResourceHandle, CopyAssignment) {
    fe::ResourceHandle<DummyMesh> original(42);
    fe::ResourceHandle<DummyMesh> copy;
    copy = original;
    EXPECT_EQ(copy.getId(), 42u);
    EXPECT_EQ(original, copy);
}

// =====================
// Hash support
// =====================

TEST(ResourceHandle, Hash_EqualHandles_SameHash) {
    fe::ResourceHandle<DummyMesh> a(10);
    fe::ResourceHandle<DummyMesh> b(10);
    std::hash<fe::ResourceHandle<DummyMesh>> hasher;
    EXPECT_EQ(hasher(a), hasher(b));
}

TEST(ResourceHandle, Hash_DifferentHandles_DifferentHash) {
    fe::ResourceHandle<DummyMesh> a(10);
    fe::ResourceHandle<DummyMesh> b(20);
    std::hash<fe::ResourceHandle<DummyMesh>> hasher;
    // Not guaranteed but extremely likely for different uint32_t values
    EXPECT_NE(hasher(a), hasher(b));
}

TEST(ResourceHandle, Hash_UsableInUnorderedSet) {
    std::unordered_set<fe::ResourceHandle<DummyMesh>> set;
    set.insert(fe::ResourceHandle<DummyMesh>(1));
    set.insert(fe::ResourceHandle<DummyMesh>(2));
    set.insert(fe::ResourceHandle<DummyMesh>(1)); // duplicate

    EXPECT_EQ(set.size(), 2u);
    EXPECT_TRUE(set.count(fe::ResourceHandle<DummyMesh>(1)));
    EXPECT_TRUE(set.count(fe::ResourceHandle<DummyMesh>(2)));
    EXPECT_FALSE(set.count(fe::ResourceHandle<DummyMesh>(3)));
}
