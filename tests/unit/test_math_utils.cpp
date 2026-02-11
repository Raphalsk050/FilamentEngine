// Unit tests for math types and camera math utilities
#include <gtest/gtest.h>
#include <filament_engine/math/types.h>

#include <cmath>

// Helper: compute direction vector from yaw/pitch (same logic as EditorCameraSystem)
static fe::Vec3 computeDirection(float yawDeg, float pitchDeg) {
    float yawRad = yawDeg * (M_PI / 180.0f);
    float pitchRad = pitchDeg * (M_PI / 180.0f);

    fe::Quat yawQuat{std::cos(yawRad / 2.0f), 0, std::sin(yawRad / 2.0f), 0};
    fe::Quat pitchQuat{std::cos(pitchRad / 2.0f), std::sin(pitchRad / 2.0f), 0, 0};
    fe::Quat rotation = yawQuat * pitchQuat;

    auto rotMat = filament::math::mat3f(rotation);
    return rotMat * fe::Vec3{0, 0, -1};
}

// Vec3 arithmetic

TEST(Vec3, DefaultInit) {
    fe::Vec3 v{0, 0, 0};
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
}

TEST(Vec3, Addition) {
    fe::Vec3 a{1, 2, 3};
    fe::Vec3 b{4, 5, 6};
    fe::Vec3 c = a + b;
    EXPECT_FLOAT_EQ(c.x, 5.0f);
    EXPECT_FLOAT_EQ(c.y, 7.0f);
    EXPECT_FLOAT_EQ(c.z, 9.0f);
}

TEST(Vec3, Subtraction) {
    fe::Vec3 a{5, 7, 9};
    fe::Vec3 b{1, 2, 3};
    fe::Vec3 c = a - b;
    EXPECT_FLOAT_EQ(c.x, 4.0f);
    EXPECT_FLOAT_EQ(c.y, 5.0f);
    EXPECT_FLOAT_EQ(c.z, 6.0f);
}

TEST(Vec3, Negation) {
    fe::Vec3 v{1, -2, 3};
    fe::Vec3 neg = -v;
    EXPECT_FLOAT_EQ(neg.x, -1.0f);
    EXPECT_FLOAT_EQ(neg.y, 2.0f);
    EXPECT_FLOAT_EQ(neg.z, -3.0f);
}

TEST(Vec3, ScalarMultiply) {
    fe::Vec3 v{1, 2, 3};
    fe::Vec3 result = v * 2.0f;
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
    EXPECT_FLOAT_EQ(result.z, 6.0f);
}

TEST(Vec3, Normalization) {
    fe::Vec3 v{3, 4, 0};
    float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    fe::Vec3 norm = v / len;
    EXPECT_NEAR(norm.x, 0.6f, 1e-5f);
    EXPECT_NEAR(norm.y, 0.8f, 1e-5f);
    EXPECT_NEAR(norm.z, 0.0f, 1e-5f);
}

TEST(Vec3, DotProduct) {
    fe::Vec3 a{1, 0, 0};
    fe::Vec3 b{0, 1, 0};
    float dot_val = dot(a, b);
    EXPECT_FLOAT_EQ(dot_val, 0.0f);

    fe::Vec3 c{1, 0, 0};
    float self_dot = dot(c, c);
    EXPECT_FLOAT_EQ(self_dot, 1.0f);
}

TEST(Vec3, CrossProduct) {
    fe::Vec3 x{1, 0, 0};
    fe::Vec3 y{0, 1, 0};
    fe::Vec3 z = cross(x, y);
    EXPECT_NEAR(z.x, 0.0f, 1e-6f);
    EXPECT_NEAR(z.y, 0.0f, 1e-6f);
    EXPECT_NEAR(z.z, 1.0f, 1e-6f);
}

TEST(Vec3, Length) {
    fe::Vec3 v{3, 4, 0};
    float len = length(v);
    EXPECT_NEAR(len, 5.0f, 1e-5f);
}

// Quaternion

TEST(Quat, Identity) {
    fe::Quat q{1, 0, 0, 0};
    // Identity quaternion applied to forward vector should give -Z
    auto rotMat = filament::math::mat3f(q);
    fe::Vec3 forward = rotMat * fe::Vec3{0, 0, -1};
    EXPECT_NEAR(forward.x, 0.0f, 1e-5f);
    EXPECT_NEAR(forward.y, 0.0f, 1e-5f);
    EXPECT_NEAR(forward.z, -1.0f, 1e-5f);
}

TEST(Quat, YawRotation_90Degrees) {
    float yaw = 90.0f * (M_PI / 180.0f);
    fe::Quat q{std::cos(yaw / 2.0f), 0, std::sin(yaw / 2.0f), 0};

    auto rotMat = filament::math::mat3f(q);
    fe::Vec3 forward = rotMat * fe::Vec3{0, 0, -1};
    EXPECT_NEAR(forward.x, -1.0f, 1e-4f);
    EXPECT_NEAR(forward.y, 0.0f, 1e-4f);
    EXPECT_NEAR(forward.z, 0.0f, 1e-4f);
}

// Camera direction (yaw/pitch)

TEST(CameraDirection, ZeroYawPitch_LooksForward) {
    fe::Vec3 dir = computeDirection(0.0f, 0.0f);
    EXPECT_NEAR(dir.x, 0.0f, 1e-4f);
    EXPECT_NEAR(dir.y, 0.0f, 1e-4f);
    EXPECT_NEAR(dir.z, -1.0f, 1e-4f);
}

TEST(CameraDirection, Yaw90_LooksLeft) {
    fe::Vec3 dir = computeDirection(90.0f, 0.0f);
    EXPECT_NEAR(dir.x, -1.0f, 1e-4f);
    EXPECT_NEAR(dir.y, 0.0f, 1e-4f);
    EXPECT_NEAR(dir.z, 0.0f, 1e-4f);
}

TEST(CameraDirection, Yaw180_LooksBack) {
    fe::Vec3 dir = computeDirection(180.0f, 0.0f);
    EXPECT_NEAR(dir.x, 0.0f, 1e-4f);
    EXPECT_NEAR(dir.y, 0.0f, 1e-4f);
    EXPECT_NEAR(dir.z, 1.0f, 1e-4f);
}

TEST(CameraDirection, PitchUp45) {
    fe::Vec3 dir = computeDirection(0.0f, 45.0f);
    EXPECT_NEAR(dir.x, 0.0f, 1e-4f);
    EXPECT_NEAR(dir.y, 0.707f, 1e-2f);
    EXPECT_NEAR(dir.z, -0.707f, 1e-2f);
}

TEST(CameraDirection, PitchDown90_LooksDown) {
    fe::Vec3 dir = computeDirection(0.0f, -89.0f);
    EXPECT_LT(dir.y, -0.99f);
    EXPECT_NEAR(dir.x, 0.0f, 1e-2f);
}

// Mat4

TEST(Mat4, Identity) {
    fe::Mat4 m = fe::Mat4(1.0f);
    EXPECT_FLOAT_EQ(m[0][0], 1.0f);
    EXPECT_FLOAT_EQ(m[1][1], 1.0f);
    EXPECT_FLOAT_EQ(m[2][2], 1.0f);
    EXPECT_FLOAT_EQ(m[3][3], 1.0f);
    // Off-diagonal should be zero
    EXPECT_FLOAT_EQ(m[0][1], 0.0f);
    EXPECT_FLOAT_EQ(m[1][0], 0.0f);
}

TEST(Mat4, Translation) {
    fe::Mat4 t = fe::Mat4::translation(fe::Vec3{1, 2, 3});
    EXPECT_FLOAT_EQ(t[3][0], 1.0f);
    EXPECT_FLOAT_EQ(t[3][1], 2.0f);
    EXPECT_FLOAT_EQ(t[3][2], 3.0f);
}

TEST(Mat4, IdentityMultiply) {
    fe::Mat4 identity = fe::Mat4(1.0f);
    fe::Mat4 t = fe::Mat4::translation(fe::Vec3{5, 10, 15});
    fe::Mat4 result = identity * t;
    EXPECT_FLOAT_EQ(result[3][0], 5.0f);
    EXPECT_FLOAT_EQ(result[3][1], 10.0f);
    EXPECT_FLOAT_EQ(result[3][2], 15.0f);
}
