#include "ecos_studio/camera_controller.hpp"
#include <algorithm>
#include <cmath>
namespace ecos::studio {
namespace {
constexpr float pi=3.14159265358979323846F;
constexpr float pitch_limit=1.55334306F;
bool finite(Vec3 v){return std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z);}
}
CameraController::CameraController(Vec3 position,float aspect,float fov,float near_plane,float far_plane,
 float move_speed,float look_sensitivity) noexcept
 :position_(position),aspect_ratio_(aspect),vertical_fov_radians_(fov),near_plane_(near_plane),
 far_plane_(far_plane),move_speed_(move_speed),look_sensitivity_(look_sensitivity){}
Result<void> CameraController::update(double delta_seconds,const InputSnapshot& input){
 if(!std::isfinite(delta_seconds)||delta_seconds<0.0||delta_seconds>1.0)
  return Result<void>::failure({ErrorCode::invalid_argument,"Camera delta time must be finite and between 0 and 1 second"});
 if(!finite(position_)||!std::isfinite(yaw_)||!std::isfinite(pitch_)||
    !std::isfinite(move_speed_)||move_speed_<0||!std::isfinite(look_sensitivity_)||look_sensitivity_<0)
  return Result<void>::failure({ErrorCode::invalid_state,"Camera controller contains invalid runtime state"});
 if(input.down(InputKey::Escape))return Result<void>::success();
 const float dt=static_cast<float>(delta_seconds);
 yaw_+=static_cast<float>(input.down(InputKey::Right))*1.8F*dt;
 yaw_-=static_cast<float>(input.down(InputKey::Left))*1.8F*dt;
 pitch_+=static_cast<float>(input.down(InputKey::Down))*1.5F*dt;
 pitch_-=static_cast<float>(input.down(InputKey::Up))*1.5F*dt;
 yaw_-=static_cast<float>(input.mouse_delta_x)*look_sensitivity_;
 pitch_-=static_cast<float>(input.mouse_delta_y)*look_sensitivity_;
 if(!std::isfinite(yaw_)||!std::isfinite(pitch_))
  return Result<void>::failure({ErrorCode::invalid_argument,"Camera look input exceeds numeric range"});
 yaw_=std::remainder(yaw_,2.0F*pi); pitch_=std::clamp(pitch_,-pitch_limit,pitch_limit);
 const float cy=std::cos(yaw_),sy=std::sin(yaw_),cp=std::cos(pitch_);
 const Vec3 forward{sy*cp,std::sin(pitch_),-cy*cp},right{cy,0,sy};
 Vec3 movement{};
 if(input.down(InputKey::W)){movement.x+=forward.x;movement.y+=forward.y;movement.z+=forward.z;}
 if(input.down(InputKey::S)){movement.x-=forward.x;movement.y-=forward.y;movement.z-=forward.z;}
 if(input.down(InputKey::D)){movement.x+=right.x;movement.z+=right.z;}
 if(input.down(InputKey::A)){movement.x-=right.x;movement.z-=right.z;}
 if(input.down(InputKey::E))movement.y+=1;
 if(input.down(InputKey::Q))movement.y-=1;
 const float magnitude=std::sqrt(movement.x*movement.x+movement.y*movement.y+movement.z*movement.z);
 if(magnitude>1.0e-7F){const float distance=move_speed_*dt/magnitude;
  position_.x+=movement.x*distance;position_.y+=movement.y*distance;position_.z+=movement.z*distance;}
 if(!finite(position_))return Result<void>::failure({ErrorCode::invalid_argument,"Camera movement produced non-finite coordinates"});
 return Result<void>::success();
}
Result<Matrix4> CameraController::view_matrix()const{
 if(!finite(position_)||!std::isfinite(yaw_)||!std::isfinite(pitch_))
  return Result<Matrix4>::failure({ErrorCode::invalid_state,"Camera state is not finite"});
 const float cp=std::cos(pitch_);
 const Vec3 forward{std::sin(yaw_)*cp,std::sin(pitch_),-std::cos(yaw_)*cp};
 return look_at(position_,{position_.x+forward.x,position_.y+forward.y,position_.z+forward.z},{0,1,0});
}
Result<Matrix4> CameraController::projection_matrix()const{
 return perspective(vertical_fov_radians_,aspect_ratio_,near_plane_,far_plane_);
}
Result<Matrix4> CameraController::view_projection_matrix()const{
 auto view=view_matrix();if(!view)return Result<Matrix4>::failure(view.error());
 auto projection=projection_matrix();if(!projection)return Result<Matrix4>::failure(projection.error());
 return Result<Matrix4>::success(multiply(projection.value(),view.value()));
}
Vec3 CameraController::position()const noexcept{return position_;}
float CameraController::yaw()const noexcept{return yaw_;}
float CameraController::pitch()const noexcept{return pitch_;}
Result<void> CameraController::set_aspect_ratio(float aspect){
 if(!std::isfinite(aspect)||aspect<=0)return Result<void>::failure({ErrorCode::invalid_argument,"Camera aspect ratio must be finite and positive"});
 aspect_ratio_=aspect;return Result<void>::success();
}
} // namespace ecos::studio
