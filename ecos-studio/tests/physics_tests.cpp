#include "ecos_studio/physics.hpp"
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
}
int main(){test_overlaps();test_ray_aabb();test_ray_sphere();if(failures){std::cerr<<failures<<" physics test(s) failed.\n";return 1;}std::cout<<"ECOS Studio collision query tests passed.\n";return 0;}
