#include "ecos_studio/input.hpp"
#include <cmath>
namespace ecos::studio {
bool InputSnapshot::down(InputKey key) const noexcept {auto i=static_cast<std::size_t>(key);return i>0&&i<keys.size()&&keys[i];}
Result<void> InputState::key_event(InputKey key,InputAction action){
 auto i=static_cast<std::size_t>(key);
 if(i==0||i>=state_.keys.size())return Result<void>::failure({ErrorCode::invalid_argument,"Invalid input key"});
 if(action!=InputAction::Press&&action!=InputAction::Release&&action!=InputAction::Repeat)return Result<void>::failure({ErrorCode::invalid_argument,"Invalid key action"});
 state_.keys[i]=action!=InputAction::Release;return Result<void>::success();
}
Result<void> InputState::mouse_button_event(std::uint8_t button,InputAction action){
 if(button>=state_.mouse_buttons.size())return Result<void>::failure({ErrorCode::invalid_argument,"Mouse button index out of range"});
 if(action!=InputAction::Press&&action!=InputAction::Release&&action!=InputAction::Repeat)return Result<void>::failure({ErrorCode::invalid_argument,"Invalid mouse action"});
 state_.mouse_buttons[button]=action!=InputAction::Release;return Result<void>::success();
}
void InputState::mouse_move(double x,double y) noexcept {
 if(!std::isfinite(x)||!std::isfinite(y))return;
 if(have_mouse_position_){state_.mouse_delta_x+=x-state_.mouse_x;state_.mouse_delta_y+=y-state_.mouse_y;}
 state_.mouse_x=x;state_.mouse_y=y;have_mouse_position_=true;
}
void InputState::scroll(double x,double y) noexcept {if(std::isfinite(x)&&std::isfinite(y)){state_.scroll_x+=x;state_.scroll_y+=y;}}
void InputState::request_quit() noexcept {state_.quit_requested=true;}
InputSnapshot InputState::snapshot() const noexcept{return state_;}
void InputState::end_frame() noexcept {state_.mouse_delta_x=state_.mouse_delta_y=state_.scroll_x=state_.scroll_y=0;}
void InputState::reset() noexcept {state_={};have_mouse_position_=false;}
}