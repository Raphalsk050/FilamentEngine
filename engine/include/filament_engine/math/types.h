#pragma once

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>
#include <math/mat4.h>
#include <math/mat3.h>
#include <math/quat.h>
#include <math/scalar.h>

namespace fe {

// Vector types
using Vec2 = filament::math::float2;
using Vec3 = filament::math::float3;
using Vec4 = filament::math::float4;

// Integer vector types
using IVec2 = filament::math::int2;
using IVec3 = filament::math::int3;
using IVec4 = filament::math::int4;

// Matrix types
using Mat3 = filament::math::mat3f;
using Mat4 = filament::math::mat4f;

// Double precision matrix (for accurate translations)
using Mat4d = filament::math::mat4;

// Quaternion
using Quat = filament::math::quatf;

} // namespace fe
