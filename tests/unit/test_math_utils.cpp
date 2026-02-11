// Unit tests for math types and camera math utilities
#include "../test_helpers.h"
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

TEST(Vec3_DefaultInit) {
    fe::Vec3 v{0, 0, 0};
    ASSERT_NEAR(v.x, 0.0f, 1e-6f);
    ASSERT_NEAR(v.y, 0.0f, 1e-6f);
    ASSERT_NEAR(v.z, 0.0f, 1e-6f);
}

TEST(Vec3_Addition) {
    fe::Vec3 a{1, 2, 3};
    fe::Vec3 b{4, 5, 6};
    fe::Vec3 c = a + b;
    ASSERT_NEAR(c.x, 5.0f, 1e-6f);
    ASSERT_NEAR(c.y, 7.0f, 1e-6f);
    ASSERT_NEAR(c.z, 9.0f, 1e-6f);
}

TEST(Vec3_ScalarMultiply) {
    fe::Vec3 v{1, 2, 3};
    fe::Vec3 result = v * 2.0f;
    ASSERT_NEAR(result.x, 2.0f, 1e-6f);
    ASSERT_NEAR(result.y, 4.0f, 1e-6f);
    ASSERT_NEAR(result.z, 6.0f, 1e-6f);
}

TEST(Vec3_Normalization) {
    fe::Vec3 v{3, 4, 0};
    float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    fe::Vec3 norm = v / len;
    ASSERT_NEAR(norm.x, 0.6f, 1e-5f);
    ASSERT_NEAR(norm.y, 0.8f, 1e-5f);
    ASSERT_NEAR(norm.z, 0.0f, 1e-5f);
}

TEST(Quat_Identity) {
    fe::Quat q{1, 0, 0, 0};
    // Identity quaternion applied to forward vector should give -Z
    auto rotMat = filament::math::mat3f(q);
    fe::Vec3 forward = rotMat * fe::Vec3{0, 0, -1};
    ASSERT_NEAR(forward.x, 0.0f, 1e-5f);
    ASSERT_NEAR(forward.y, 0.0f, 1e-5f);
    ASSERT_NEAR(forward.z, -1.0f, 1e-5f);
}

TEST(Quat_YawRotation_90Degrees) {
    // 90 degree yaw with Filament's convention
    float yaw = 90.0f * (M_PI / 180.0f);
    fe::Quat q{std::cos(yaw / 2.0f), 0, std::sin(yaw / 2.0f), 0};

    auto rotMat = filament::math::mat3f(q);
    fe::Vec3 forward = rotMat * fe::Vec3{0, 0, -1};
    // Filament quaternion: yaw 90 around Y rotates -Z toward -X
    ASSERT_NEAR(forward.x, -1.0f, 1e-4f);
    ASSERT_NEAR(forward.y, 0.0f, 1e-4f);
    ASSERT_NEAR(forward.z, 0.0f, 1e-4f);
}

TEST(CameraDirection_ZeroYawPitch_LooksForward) {
    fe::Vec3 dir = computeDirection(0.0f, 0.0f);
    ASSERT_NEAR(dir.x, 0.0f, 1e-4f);
    ASSERT_NEAR(dir.y, 0.0f, 1e-4f);
    ASSERT_NEAR(dir.z, -1.0f, 1e-4f);
}

TEST(CameraDirection_Yaw90_LooksLeft) {
    // Filament: yaw 90 with our quaternion construction goes to -X
    fe::Vec3 dir = computeDirection(90.0f, 0.0f);
    ASSERT_NEAR(dir.x, -1.0f, 1e-4f);
    ASSERT_NEAR(dir.y, 0.0f, 1e-4f);
    ASSERT_NEAR(dir.z, 0.0f, 1e-4f);
}

TEST(CameraDirection_PitchUp45) {
    fe::Vec3 dir = computeDirection(0.0f, 45.0f);
    // Looking up at 45 degrees: y ~= 0.707, z ~= -0.707
    ASSERT_NEAR(dir.x, 0.0f, 1e-4f);
    ASSERT_NEAR(dir.y, 0.707f, 1e-2f);
    ASSERT_NEAR(dir.z, -0.707f, 1e-2f);
}

TEST(CameraDirection_PitchDown90_LooksDown) {
    fe::Vec3 dir = computeDirection(0.0f, -89.0f);
    // Almost straight down
    ASSERT_LT(dir.y, -0.99f);
    ASSERT_NEAR(dir.x, 0.0f, 1e-2f);
}

TEST(Mat4_Identity) {
    fe::Mat4 m = fe::Mat4(1.0f); // identity
    // Check diagonal is 1
    ASSERT_NEAR(m[0][0], 1.0f, 1e-6f);
    ASSERT_NEAR(m[1][1], 1.0f, 1e-6f);
    ASSERT_NEAR(m[2][2], 1.0f, 1e-6f);
    ASSERT_NEAR(m[3][3], 1.0f, 1e-6f);
}

TEST(Mat4_Translation) {
    fe::Mat4 t = fe::Mat4::translation(fe::Vec3{1, 2, 3});
    ASSERT_NEAR(t[3][0], 1.0f, 1e-6f);
    ASSERT_NEAR(t[3][1], 2.0f, 1e-6f);
    ASSERT_NEAR(t[3][2], 3.0f, 1e-6f);
}

int main() {
    return runAllTests();
}
