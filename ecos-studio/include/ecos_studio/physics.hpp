#pragma once

#include "ecos_studio/result.hpp"
#include "ecos_studio/scene.hpp"

namespace ecos::studio {

struct Aabb {
    Vec3 minimum{};
    Vec3 maximum{};
};
struct Sphere {
    Vec3 center{};
    float radius{1.0F};
};
struct Ray {
    Vec3 origin{};
    Vec3 direction{};
};
struct RayHit {
    bool hit{false};
    float distance{0.0F};
    Vec3 position{};
    Vec3 normal{};
};

[[nodiscard]] bool overlaps(const Aabb& a, const Aabb& b) noexcept;
[[nodiscard]] bool overlaps(const Sphere& a, const Sphere& b) noexcept;
[[nodiscard]] Result<RayHit> intersect_ray_aabb(const Ray& ray, const Aabb& bounds,
                                                float max_distance = 3.402823466e+38F);
[[nodiscard]] Result<RayHit> intersect_ray_sphere(const Ray& ray, const Sphere& sphere,
                                                  float max_distance = 3.402823466e+38F);

} // namespace ecos::studio
