#pragma once

#include "ecos_studio/entity.hpp"
#include "ecos_studio/result.hpp"
#include "ecos_studio/scene.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
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
    // Coefficient of restitution: 0 is inelastic, 1 is perfectly elastic.
    float restitution{0.0F};
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
    // Coulomb friction coefficients used by rigid-body contact resolution.
    float static_friction{0.6F};
    float dynamic_friction{0.4F};
};

struct ColliderRayHit {
    Entity entity{};
    RayHit hit{};
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

enum class CollisionPhase { enter, stay, exit };

struct CollisionEvent {
    CollisionPhase phase{CollisionPhase::enter};
    CollisionPair pair{};
};

// Maintains overlap state between simulation ticks and emits deterministic
// enter/stay/exit transitions. A tracker belongs to one active simulation world.
class CollisionEventTracker {
public:
    [[nodiscard]] Result<std::vector<CollisionEvent>> update(EntityRegistry& entities);
    void reset() noexcept { active_pairs_.clear(); }

private:
    std::vector<CollisionPair> active_pairs_;
};

// World-space ray query against ECS colliders. A missing hit is a successful empty result.
class PhysicsQuerySystem {
public:
    [[nodiscard]] static Result<std::optional<ColliderRayHit>> raycast(
        EntityRegistry& entities, const Ray& ray,
        float max_distance = 3.402823466e+38F,
        std::uint32_t layer_mask = 0xFFFFFFFFU,
        bool include_triggers = true);
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
    // Resolves non-trigger contacts with positional correction and removes
    // inward normal velocity. Dynamic bodies must be root scene nodes so
    // world-space corrections cannot corrupt parent-local transforms.
    [[nodiscard]] static Result<std::size_t> resolve_collisions(
        EntityRegistry& entities, std::size_t iterations = 4U);
};

[[nodiscard]] bool overlaps(const Aabb& a, const Aabb& b) noexcept;
[[nodiscard]] bool overlaps(const Sphere& a, const Sphere& b) noexcept;
[[nodiscard]] Result<RayHit> intersect_ray_aabb(const Ray& ray, const Aabb& bounds,
                                                float max_distance = 3.402823466e+38F);
[[nodiscard]] Result<RayHit> intersect_ray_sphere(const Ray& ray, const Sphere& sphere,
                                                  float max_distance = 3.402823466e+38F);

} // namespace ecos::studio
