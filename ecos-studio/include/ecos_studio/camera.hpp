#pragma once

#include "ecos_studio/result.hpp"
#include "ecos_studio/scene.hpp"

namespace ecos::studio {

struct Vec4 {
    float x{0.0F};
    float y{0.0F};
    float z{0.0F};
    float w{0.0F};
};

// Matrices use the same row-major storage and column-vector convention as scene transforms.
[[nodiscard]] Result<Matrix4> look_at(Vec3 eye, Vec3 target, Vec3 up);
[[nodiscard]] Result<Matrix4> perspective(float vertical_fov_radians, float aspect_ratio,
                                         float near_plane, float far_plane);
[[nodiscard]] Result<Matrix4> orthographic(float left, float right, float bottom, float top,
                                           float near_plane, float far_plane);
[[nodiscard]] Vec4 transform(const Matrix4& matrix, Vec4 vector) noexcept;
[[nodiscard]] Result<Vec3> project_to_ndc(const Matrix4& view_projection, Vec3 world_position);

} // namespace ecos::studio
