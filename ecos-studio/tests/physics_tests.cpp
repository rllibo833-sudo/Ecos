#include "ecos_studio/physics.hpp"
#include "ecos_studio/world_content_runtime.hpp"
#include <cmath>
#include <iostream>
namespace {
int failures=0;
void check(bool v,const char* m){if(!v){std::cerr<<"FAIL: "<<m<<'\n';++failures;}}
bool near(float a,float b){return std::abs(a-b)<1.0e-4F;}
void test_overlaps(){
 using namespace ecos::studio;
 check(overlaps(Aabb{{0,0,0},{1,1,1}},Aabb{{1,1,1},{2,2,2}}),"AABBs touching at a boundary overlap");
 check(!overlaps(Aabb{{0,0,0},{1,1,1}},Aabb{{2,0,0},{3,1,1}}),"separated AABBs do not overlap");
 check(overlaps(Sphere{{0,0,0},1},Sphere{{2,0,0},1}),"touching spheres overlap");
 check(!overlaps(Sphere{{0,0,0},1},Sphere{{2.1F,0,0},1}),"separated spheres do not overlap");
}
void test_ray_aabb(){
 using namespace ecos::studio;
 auto hit=intersect_ray_aabb({{-3,0,0},{2,0,0}},Aabb{{-1,-1,-1},{1,1,1}});
 check(hit&&hit.value().hit,"ray hits AABB with non-unit direction");
 if(hit){check(near(hit.value().distance,2)&&near(hit.value().position.x,-1),"returns world distance and hit position");check(hit.value().normal.x==-1,"returns entry-face normal");}
 auto miss=intersect_ray_aabb({{-3,3,0},{1,0,0}},Aabb{{-1,-1,-1},{1,1,1}});
 check(miss&&!miss.value().hit,"parallel ray outside slab misses");
 auto inside=intersect_ray_aabb({{0,0,0},{1,0,0}},Aabb{{-1,-1,-1},{1,1,1}});
 check(inside&&inside.value().hit&&near(inside.value().distance,1),"inside-origin ray returns exit hit");
 check(!intersect_ray_aabb({{-3,0,0},{0,0,0}},Aabb{{-1,-1,-1},{1,1,1}}),"rejects zero direction");
}
void test_ray_sphere(){
 using namespace ecos::studio;
 auto hit=intersect_ray_sphere({{-3,0,0},{1,0,0}},Sphere{{0,0,0},1});
 check(hit&&hit.value().hit&&near(hit.value().distance,2),"ray intersects sphere");
 if(hit)check(near(hit.value().normal.x,-1),"sphere hit has outward normal");
 auto miss=intersect_ray_sphere({{-3,3,0},{1,0,0}},Sphere{{0,0,0},1});
 check(miss&&!miss.value().hit,"ray misses sphere");
 auto limited=intersect_ray_sphere({{-3,0,0},{1,0,0}},Sphere{{0,0,0},1},1.0F);
 check(limited&&!limited.value().hit,"respects maximum ray distance");
 check(!intersect_ray_sphere({{0,0,0},{1,0,0},},Sphere{{0,0,0},-1}),"rejects negative sphere radius");
}
void test_ecs_physics_integration(){
 using namespace ecos::studio;
 EntityRegistry entities;
 const auto falling=entities.create();
 Transform transform; transform.position={0,10,0};
 entities.emplace<SceneNodeComponent>(falling,SceneNodeComponent{"falling","",transform,{},{}});
 entities.emplace<RigidBodyComponent>(falling,RigidBodyComponent{});
 auto stepped=PhysicsSystem::step(entities,0.5);
 check(stepped&&stepped.value()==1,"integrates dynamic ECS body");
 auto* node=entities.get<SceneNodeComponent>(falling);
 auto* body=entities.get<RigidBodyComponent>(falling);
 check(node&&body&&near(body->velocity.y,-4.905F)&&near(node->local_transform.position.y,7.5475F),
       "gravity updates velocity and scene-local position in one fixed step");

 const auto fixed=entities.create();
 Transform fixed_transform; fixed_transform.position={2,3,4};
 entities.emplace<SceneNodeComponent>(fixed,SceneNodeComponent{"fixed","",fixed_transform,{},{}});
 RigidBodyComponent fixed_body; fixed_body.kinematic=true; fixed_body.velocity={10,10,10};
 entities.emplace<RigidBodyComponent>(fixed,fixed_body);
 stepped=PhysicsSystem::step(entities,1.0);
 check(stepped&&stepped.value()==1,"kinematic body is skipped while dynamic body advances");
 check(near(entities.get<SceneNodeComponent>(fixed)->local_transform.position.y,3)&&
       near(entities.get<RigidBodyComponent>(fixed)->velocity.y,10),"kinematic body preserves pose and velocity");

 const float before=node->local_transform.position.y;
 entities.get<RigidBodyComponent>(fixed)->linear_damping=-1;
 stepped=PhysicsSystem::step(entities,0.25);
 check(!stepped&&near(node->local_transform.position.y,before),
       "invalid body aborts the whole physics step without partial movement");
 check(!PhysicsSystem::step(entities,-0.1),"rejects negative delta time");
}
}
int main(){test_overlaps();test_ray_aabb();test_ray_sphere();test_ecs_physics_integration();if(failures){std::cerr<<failures<<" physics test(s) failed.\n";return 1;}std::cout<<"ECOS Studio collision and ECS physics tests passed.\n";return 0;}
