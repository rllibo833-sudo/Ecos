#pragma once
#include "ecos_studio/camera.hpp"
#include "ecos_studio/input.hpp"
namespace ecos::studio {
class CameraController {
public:
 explicit CameraController(Vec3 position={0.0F,0.0F,3.0F}, float aspect_ratio=16.0F/9.0F,
  float vertical_fov_radians=1.04719755F, float near_plane=0.1F, float far_plane=1000.0F,
  float move_speed=5.0F, float look_sensitivity=0.0025F) noexcept;
 [[nodiscard]] Result<void> update(double delta_seconds,const InputSnapshot& input);
 [[nodiscard]] Result<Matrix4> view_matrix() const;
 [[nodiscard]] Result<Matrix4> projection_matrix() const;
 [[nodiscard]] Result<Matrix4> view_projection_matrix() const;
 [[nodiscard]] Vec3 position() const noexcept;
 [[nodiscard]] float yaw() const noexcept;
 [[nodiscard]] float pitch() const noexcept;
 [[nodiscard]] Result<void> set_aspect_ratio(float aspect_ratio);
private:
 Vec3 position_{}; float yaw_{0.0F}; float pitch_{0.0F};
 float aspect_ratio_{16.0F/9.0F}; float vertical_fov_radians_{1.04719755F};
 float near_plane_{0.1F}; float far_plane_{1000.0F}; float move_speed_{5.0F};
 float look_sensitivity_{0.0025F};
};
} // namespace ecos::studio
