#include "ecos_studio/physics.hpp"
#include "ecos_studio/world_content_runtime.hpp"
#include "ecos_studio/world_transform.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace ecos::studio {
namespace {
bool finite(Vec3 v) { return std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z); }
Vec3 add(Vec3 a,Vec3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
Vec3 subtract(Vec3 a,Vec3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
Vec3 scale(Vec3 v,float s){return {v.x*s,v.y*s,v.z*s};}
float dot(Vec3 a,Vec3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
float length(Vec3 v){return std::sqrt(dot(v,v));}
bool valid_ray(const Ray& ray,float max_distance) {
    return finite(ray.origin)&&finite(ray.direction)&&length(ray.direction)>1.0e-7F&&
        !std::isnan(max_distance)&&max_distance>=0.0F;
}
bool valid_aabb(const Aabb& b) {
    return finite(b.minimum)&&finite(b.maximum)&&b.minimum.x<=b.maximum.x&&
        b.minimum.y<=b.maximum.y&&b.minimum.z<=b.maximum.z;
}
bool valid_sphere(const Sphere& s) { return finite(s.center)&&std::isfinite(s.radius)&&s.radius>=0.0F; }
struct BodyUpdate {
    Entity entity;
    Vec3 velocity;
    Vec3 position;
};
}

Result<std::size_t> PhysicsSystem::step(EntityRegistry& entities, double delta_seconds, Vec3 gravity) {
    if (!std::isfinite(delta_seconds) || delta_seconds < 0.0 ||
        delta_seconds > static_cast<double>(std::numeric_limits<float>::max()) || !finite(gravity)) {
        return Result<std::size_t>::failure(
            {ErrorCode::invalid_argument, "Physics step has an invalid delta time or gravity"});
    }
    const float dt = static_cast<float>(delta_seconds);
    const auto bodies = entities.entities_with<RigidBodyComponent>();
    std::vector<BodyUpdate> updates;
    updates.reserve(bodies.size());

    // Validate and calculate every result before publishing any mutation.
    for (const auto entity : bodies) {
        const auto* body = entities.get<RigidBodyComponent>(entity);
        const auto* node = entities.get<SceneNodeComponent>(entity);
        if (!body || !node) {
            return Result<std::size_t>::failure(
                {ErrorCode::invalid_state, "Rigid body requires a live scene-node component"});
        }
        if (!finite(body->velocity) || !finite(body->acceleration) ||
            !std::isfinite(body->gravity_scale) || !std::isfinite(body->linear_damping) ||
            body->linear_damping < 0.0F || !finite(node->local_transform.position)) {
            return Result<std::size_t>::failure(
                {ErrorCode::invalid_argument, "Rigid body contains non-finite values or negative damping"});
        }
        if (body->kinematic || dt == 0.0F) continue;

        const Vec3 total_acceleration = add(body->acceleration, scale(gravity, body->gravity_scale));
        Vec3 velocity = add(body->velocity, scale(total_acceleration, dt));
        // Exponential damping is stable across varying fixed-step sizes.
        velocity = scale(velocity, std::exp(-body->linear_damping * dt));
        const Vec3 position = add(node->local_transform.position, scale(velocity, dt));
        if (!finite(velocity) || !finite(position)) {
            return Result<std::size_t>::failure(
                {ErrorCode::invalid_argument, "Physics integration overflowed to a non-finite value"});
        }
        updates.push_back({entity, velocity, position});
    }

    for (const auto& update : updates) {
        entities.get<RigidBodyComponent>(update.entity)->velocity = update.velocity;
        entities.get<SceneNodeComponent>(update.entity)->local_transform.position = update.position;
    }
    return Result<std::size_t>::success(updates.size());
}


namespace {
struct WorldCollider {
    Entity entity;
    ColliderShape shape;
    Vec3 center;
    Vec3 half_extents;
    float radius;
    std::uint32_t layer;
    std::uint32_t mask;
    bool trigger;
};
float component_scale_x(const Matrix4& m) {
    return std::sqrt(m.values[0]*m.values[0]+m.values[4]*m.values[4]+m.values[8]*m.values[8]);
}
float component_scale_y(const Matrix4& m) {
    return std::sqrt(m.values[1]*m.values[1]+m.values[5]*m.values[5]+m.values[9]*m.values[9]);
}
float component_scale_z(const Matrix4& m) {
    return std::sqrt(m.values[2]*m.values[2]+m.values[6]*m.values[6]+m.values[10]*m.values[10]);
}
bool valid_collider(const ColliderComponent& c) {
    return std::isfinite(c.radius) && c.radius >= 0.0F &&
        finite(c.half_extents) && c.half_extents.x >= 0.0F &&
        c.half_extents.y >= 0.0F && c.half_extents.z >= 0.0F &&
        (c.shape == ColliderShape::sphere || c.shape == ColliderShape::box);
}
bool contact(const WorldCollider& a, const WorldCollider& b, Vec3& normal, float& penetration) {
    if (a.shape == ColliderShape::sphere && b.shape == ColliderShape::sphere) {
        const Vec3 delta = subtract(b.center, a.center);
        const float distance_squared = dot(delta, delta);
        const float radii = a.radius + b.radius;
        if (distance_squared > radii*radii) return false;
        const float distance = std::sqrt(std::max(0.0F, distance_squared));
        normal = distance > 1.0e-7F ? scale(delta, 1.0F/distance) : Vec3{1.0F,0.0F,0.0F};
        penetration = radii - distance;
        return true;
    }
    const Vec3 a_min = subtract(a.center, a.half_extents);
    const Vec3 a_max = add(a.center, a.half_extents);
    const Vec3 b_min = subtract(b.center, b.half_extents);
    const Vec3 b_max = add(b.center, b.half_extents);
    if (a.shape == ColliderShape::box && b.shape == ColliderShape::box) {
        const float ox = std::min(a_max.x,b_max.x)-std::max(a_min.x,b_min.x);
        const float oy = std::min(a_max.y,b_max.y)-std::max(a_min.y,b_min.y);
        const float oz = std::min(a_max.z,b_max.z)-std::max(a_min.z,b_min.z);
        if (ox < 0.0F || oy < 0.0F || oz < 0.0F) return false;
        penetration = ox; normal = {b.center.x >= a.center.x ? 1.0F : -1.0F,0,0};
        if (oy < penetration) { penetration=oy; normal={0,b.center.y >= a.center.y ? 1.0F : -1.0F,0}; }
        if (oz < penetration) { penetration=oz; normal={0,0,b.center.z >= a.center.z ? 1.0F : -1.0F}; }
        return true;
    }
    const bool sphere_first = a.shape == ColliderShape::sphere;
    const auto& sphere = sphere_first ? a : b;
    const auto& box = sphere_first ? b : a;
    const Vec3 box_min = subtract(box.center, box.half_extents);
    const Vec3 box_max = add(box.center, box.half_extents);
    const Vec3 closest{
        std::clamp(sphere.center.x,box_min.x,box_max.x),
        std::clamp(sphere.center.y,box_min.y,box_max.y),
        std::clamp(sphere.center.z,box_min.z,box_max.z)};
    Vec3 delta = subtract(closest,sphere.center);
    float distance = length(delta);
    if (distance > sphere.radius) return false;
    if (distance > 1.0e-7F) {
        normal = scale(delta,1.0F/distance);
        penetration = sphere.radius-distance;
    } else {
        const float dx=std::min(sphere.center.x-box_min.x,box_max.x-sphere.center.x);
        const float dy=std::min(sphere.center.y-box_min.y,box_max.y-sphere.center.y);
        const float dz=std::min(sphere.center.z-box_min.z,box_max.z-sphere.center.z);
        if (dx<=dy && dx<=dz) { normal={sphere.center.x>=box.center.x ? 1.0F : -1.0F,0,0}; penetration=sphere.radius+dx; }
        else if (dy<=dz) { normal={0,sphere.center.y>=box.center.y ? 1.0F : -1.0F,0}; penetration=sphere.radius+dy; }
        else { normal={0,0,sphere.center.z>=box.center.z ? 1.0F : -1.0F}; penetration=sphere.radius+dz; }
    }
    if (sphere_first) normal=scale(normal,-1.0F);
    return true;
}
}

Result<std::vector<CollisionPair>> CollisionSystem::detect(EntityRegistry& entities) {
    auto transformed = WorldTransformSystem::update(entities);
    if (!transformed) return Result<std::vector<CollisionPair>>::failure(transformed.error());
    std::vector<WorldCollider> colliders;
    const auto collider_entities = entities.entities_with<ColliderComponent>();
    colliders.reserve(collider_entities.size());
    for (const auto entity : collider_entities) {
        const auto* collider = entities.get<ColliderComponent>(entity);
        const auto* world = entities.get<WorldTransformComponent>(entity);
        if (!collider || !world || !valid_collider(*collider)) {
            return Result<std::vector<CollisionPair>>::failure(
                {ErrorCode::invalid_argument, "Collision entity has invalid collider data or no world transform"});
        }
        const auto& m = world->matrix.values;
        const Vec3 center{m[3],m[7],m[11]};
        const Vec3 scales{component_scale_x(world->matrix),component_scale_y(world->matrix),component_scale_z(world->matrix)};
        if (!finite(center) || !finite(scales)) {
            return Result<std::vector<CollisionPair>>::failure(
                {ErrorCode::invalid_argument, "Collision transform contains non-finite values"});
        }
        Vec3 extents{collider->half_extents.x*scales.x,collider->half_extents.y*scales.y,collider->half_extents.z*scales.z};
        float radius=collider->radius*std::max({scales.x,scales.y,scales.z});
        if (collider->shape == ColliderShape::box) {
            // Transform the local box basis to a conservative world AABB,
            // including rotation and non-uniform scale.
            extents={
                std::abs(m[0])*collider->half_extents.x+std::abs(m[1])*collider->half_extents.y+std::abs(m[2])*collider->half_extents.z,
                std::abs(m[4])*collider->half_extents.x+std::abs(m[5])*collider->half_extents.y+std::abs(m[6])*collider->half_extents.z,
                std::abs(m[8])*collider->half_extents.x+std::abs(m[9])*collider->half_extents.y+std::abs(m[10])*collider->half_extents.z};
        }
        colliders.push_back({entity,collider->shape,center,extents,radius,collider->layer,collider->mask,collider->trigger});
    }
    std::vector<CollisionPair> pairs;
    for (std::size_t i=0;i<colliders.size();++i) {
        for (std::size_t j=i+1;j<colliders.size();++j) {
            const auto& a=colliders[i]; const auto& b=colliders[j];
            if ((a.mask & b.layer)==0U || (b.mask & a.layer)==0U) continue;
            Vec3 normal{}; float penetration=0.0F;
            if (contact(a,b,normal,penetration))
                pairs.push_back({a.entity,b.entity,normal,penetration,a.trigger||b.trigger});
        }
    }
    return Result<std::vector<CollisionPair>>::success(std::move(pairs));
}

bool overlaps(const Aabb& a,const Aabb& b) noexcept {
    return valid_aabb(a)&&valid_aabb(b)&&
        a.minimum.x<=b.maximum.x&&a.maximum.x>=b.minimum.x&&
        a.minimum.y<=b.maximum.y&&a.maximum.y>=b.minimum.y&&
        a.minimum.z<=b.maximum.z&&a.maximum.z>=b.minimum.z;
}
bool overlaps(const Sphere& a,const Sphere& b) noexcept {
    if(!valid_sphere(a)||!valid_sphere(b))return false;
    const auto d=subtract(a.center,b.center);
    const float radius=a.radius+b.radius;
    return dot(d,d)<=radius*radius;
}
Result<RayHit> intersect_ray_aabb(const Ray& ray,const Aabb& b,float max_distance) {
    if(!valid_ray(ray,max_distance)||!valid_aabb(b))
        return Result<RayHit>::failure({ErrorCode::invalid_argument,"Ray or AABB contains invalid values"});
    const float direction_length=length(ray.direction);
    const Vec3 d=scale(ray.direction,1.0F/direction_length);
    float near_t=-std::numeric_limits<float>::infinity();
    float far_t=std::numeric_limits<float>::infinity();
    Vec3 near_normal{},far_normal{};
    const float origins[3]={ray.origin.x,ray.origin.y,ray.origin.z};
    const float directions[3]={d.x,d.y,d.z};
    const float mins[3]={b.minimum.x,b.minimum.y,b.minimum.z};
    const float maxs[3]={b.maximum.x,b.maximum.y,b.maximum.z};
    for(int axis=0;axis<3;++axis){
        if(std::abs(directions[axis])<=1.0e-8F){
            if(origins[axis]<mins[axis]||origins[axis]>maxs[axis])
                return Result<RayHit>::success({});
            continue;
        }
        float t1=(mins[axis]-origins[axis])/directions[axis];
        float t2=(maxs[axis]-origins[axis])/directions[axis];
        float n1=-1.0F,n2=1.0F;
        if(t1>t2){std::swap(t1,t2);std::swap(n1,n2);}
        if(t1>near_t){near_t=t1;near_normal={0,0,0};if(axis==0)near_normal.x=n1;if(axis==1)near_normal.y=n1;if(axis==2)near_normal.z=n1;}
        if(t2<far_t){far_t=t2;far_normal={0,0,0};if(axis==0)far_normal.x=n2;if(axis==1)far_normal.y=n2;if(axis==2)far_normal.z=n2;}
        if(near_t>far_t)return Result<RayHit>::success({});
    }
    const bool inside=near_t<0.0F;
    const float distance=inside?far_t:near_t;
    const Vec3 normal=inside?far_normal:near_normal;
    if(distance<0.0F||distance>max_distance||!std::isfinite(distance))return Result<RayHit>::success({});
    return Result<RayHit>::success({true,distance,add(ray.origin,scale(d,distance)),normal});
}
Result<RayHit> intersect_ray_sphere(const Ray& ray,const Sphere& sphere,float max_distance) {
    if(!valid_ray(ray,max_distance)||!valid_sphere(sphere))
        return Result<RayHit>::failure({ErrorCode::invalid_argument,"Ray or sphere contains invalid values"});
    const float direction_length=length(ray.direction);
    const Vec3 d=scale(ray.direction,1.0F/direction_length);
    const Vec3 offset=subtract(ray.origin,sphere.center);
    const float b=dot(offset,d);
    const float c=dot(offset,offset)-sphere.radius*sphere.radius;
    const float discriminant=b*b-c;
    if(discriminant<0.0F)return Result<RayHit>::success({});
    const float root=std::sqrt(discriminant);
    float distance=-b-root;
    if(distance<0.0F)distance=-b+root;
    if(distance<0.0F||distance>max_distance||!std::isfinite(distance))return Result<RayHit>::success({});
    const Vec3 position=add(ray.origin,scale(d,distance));
    const Vec3 radial=subtract(position,sphere.center);
    const float radial_length=length(radial);
    const Vec3 normal=radial_length>1.0e-7F?scale(radial,1.0F/radial_length):scale(d,-1.0F);
    return Result<RayHit>::success({true,distance,position,normal});
}
} // namespace ecos::studio
