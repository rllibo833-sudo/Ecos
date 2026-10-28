#include "ecos_studio/camera.hpp"
#include <cmath>
#include <iostream>
namespace {
int failures=0;
void check(bool ok,const char* message){if(!ok){std::cerr<<"FAIL: "<<message<<'\n';++failures;}}
bool near(float a,float b){return std::abs(a-b)<1.0e-4F;}
void test_view_matrix(){
 using namespace ecos::studio;
 auto view=look_at({0,0,0},{0,0,-1},{0,1,0});
 check(static_cast<bool>(view),"constructs a view matrix");
 if(view){auto p=transform(view.value(),{0,0,-4,1});check(near(p.z,-4)&&near(p.w,1),"view maps forward points into camera space");}
 check(!look_at({0,0,0},{0,0,0},{0,1,0}),"rejects coincident eye and target");
 check(!look_at({0,0,0},{0,1,0},{0,2,0}),"rejects parallel up vector");
}
void test_perspective(){
 using namespace ecos::studio;
 auto projection=perspective(1.04719755F,16.0F/9.0F,0.1F,1000.0F);
 check(static_cast<bool>(projection),"constructs perspective projection");
 if(projection){
  auto center=project_to_ndc(projection.value(),{0,0,-1});
  check(center&&near(center.value().x,0)&&near(center.value().y,0),"projects optical-axis point to screen center");
  auto near_point=project_to_ndc(projection.value(),{0,0,-0.1F});
  check(near_point&&near(near_point.value().z,-1),"maps near plane to OpenGL NDC -1");
 }
 check(!perspective(0,1,0.1F,100),"rejects zero FOV");
 check(!perspective(1,0,0.1F,100),"rejects zero aspect ratio");
 check(!perspective(1,1,100,10),"rejects reversed clipping planes");
}
void test_orthographic(){
 using namespace ecos::studio;
 auto projection=orthographic(-2,2,-1,1,0.1F,10);
 check(static_cast<bool>(projection),"constructs orthographic projection");
 if(projection){
  auto center=project_to_ndc(projection.value(),{0,0,-1});
  check(center&&near(center.value().x,0)&&near(center.value().y,0),"maps orthographic center to NDC center");
 }
 check(!orthographic(1,1,-1,1,0.1F,10),"rejects degenerate horizontal bounds");
}
}
int main(){test_view_matrix();test_perspective();test_orthographic();if(failures){std::cerr<<failures<<" camera test(s) failed.\n";return 1;}std::cout<<"ECOS Studio camera math tests passed.\n";return 0;}
