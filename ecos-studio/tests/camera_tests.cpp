#include "ecos_studio/camera.hpp"
#include "ecos_studio/camera_controller.hpp"
#include "ecos_studio/engine_loop.hpp"
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

void test_input_driven_camera_runtime(){
 using namespace ecos::studio;
 CameraController camera({0,0,3},1.5F);
 const auto start=camera.position();
 InputSnapshot input; input.keys[static_cast<std::size_t>(InputKey::W)]=true;
 auto moved=camera.update(0.5,input);
 check(static_cast<bool>(moved),"updates runtime camera from movement input");
 const auto after=camera.position();
 check(std::abs(after.z-(start.z-2.5F))<1.0e-4F,"W moves camera forward at configured units per second");
 input={}; input.keys[static_cast<std::size_t>(InputKey::Right)]=true;
 const float old_yaw=camera.yaw();
 check(static_cast<bool>(camera.update(0.25,input)),"updates yaw from arrow-key input");
 check(camera.yaw()>old_yaw,"right arrow rotates camera yaw");
 input={}; input.mouse_delta_x=100.0; input.mouse_delta_y=-10000.0;
 const float yaw_before_mouse=camera.yaw();
 check(static_cast<bool>(camera.update(0.01,input)),"accepts mouse look delta");
 check(camera.yaw()<yaw_before_mouse,"mouse horizontal delta changes camera heading");
 check(camera.pitch()<1.56F&&camera.pitch()>-1.56F,"mouse look clamps pitch away from singularity");
 auto vp=camera.view_projection_matrix();
 check(static_cast<bool>(vp),"produces a view-projection matrix from runtime camera state");
 check(!camera.set_aspect_ratio(0.0F),"rejects zero camera aspect ratio");
 check(!camera.update(-0.1,InputSnapshot{}),"rejects negative frame delta");
 check(!camera.update(2.0,InputSnapshot{}),"rejects unbounded frame delta");

 EngineLoop loop;
 CameraController loop_camera({0,0,3},1.5F);
 check(static_cast<bool>(loop.input().key_event(InputKey::W,InputAction::Press)),"queues camera movement in engine input state");
 FrameCallbacks callbacks;
 callbacks.fixed_update=[&](double dt,const InputSnapshot& snapshot){
  auto result=loop_camera.update(dt,snapshot);
  check(static_cast<bool>(result),"engine fixed update drives runtime camera");
 };
 const auto report=loop.frame(1.0/60.0,callbacks);
 check(report.update_callbacks==1&&loop_camera.position().z<3.0F,
  "engine frame input moves camera during fixed update");
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
int main(){test_view_matrix();test_perspective();test_orthographic();test_input_driven_camera_runtime();if(failures){std::cerr<<failures<<" camera test(s) failed.\n";return 1;}std::cout<<"ECOS Studio camera math tests passed.\n";return 0;}
