#pragma once

#include "ecos_studio/entity.hpp"
#include "ecos_studio/result.hpp"
#include "ecos_studio/scene.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

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

// Dynamic-body data consumed by PhysicsSystem on each fixed simulation step.
// Acceleration is persistent; set it to zero when no longer required.
struct RigidBodyComponent {
    Vec3 velocity{};
    Vec3 acceleration{};
    float gravity_scale{1.0F};
    float linear_damping{0.0F};
    bool kinematic{false};
};

struct SceneNodeComponent;

enum class ColliderShape { sphere, box };

struct ColliderComponent {
    ColliderShape shape{ColliderShape::sphere};
    // Sphere radius or box half-extents in local space.
    float radius{0.5F};
    Vec3 half_extents{0.5F, 0.5F, 0.5F};
    std::uint32_t layer{1U};
    std::uint32_t mask{0xFFFFFFFFU};
    bool trigger{false};
};

struct CollisionPair {
    Entity first{};
    Entity second{};
    // Contact normal points from first toward second.
    Vec3 normal{};
    float penetration{0.0F};
    bool trigger{false};
};

class CollisionSystem {
public:
    // Updates hierarchical transforms, then returns deterministic overlapping
    // pairs. Layer/mask filtering is symmetric; triggers report but don't resolve.
    [[nodiscard]] static Result<std::vector<CollisionPair>> detect(EntityRegistry& entities);
};

// Integrates ECS rigid bodies into scene-local transforms. Validation is
// transactional: invalid body data prevents every body from being modified.
class PhysicsSystem {
public:
    [[nodiscard]] static Result<std::size_t> step(
        EntityRegistry& entities, double delta_seconds,
        Vec3 gravity = {0.0F, -9.81F, 0.0F});
};

[[nodiscard]] bool overlaps(const Aabb& a, const Aabb& b) noexcept;
[[nodiscard]] bool overlaps(const Sphere& a, const Sphere& b) noexcept;
[[nodiscard]] Result<RayHit> intersect_ray_aabb(const Ray& ray, const Aabb& bounds,
                                                float max_distance = 3.402823466e+38F);
[[nodiscard]] Result<RayHit> intersect_ray_sphere(const Ray& ray, const Sphere& sphere,
                                                  float max_distance = 3.402823466e+38F);

} // namespace ecos::studio
