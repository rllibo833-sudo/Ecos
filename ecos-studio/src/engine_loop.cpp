#include "ecos_studio/engine_loop.hpp"
namespace ecos::studio {
EngineLoop::EngineLoop(double fixed_delta_seconds,double max_frame_delta_seconds,std::size_t max_fixed_steps) noexcept
 :clock_(fixed_delta_seconds,max_frame_delta_seconds,max_fixed_steps){}
FrameReport EngineLoop::frame(double elapsed_seconds,const FrameCallbacks& callbacks){
 FrameReport report;
 report.timing=clock_.advance(elapsed_seconds);
 const auto input_snapshot=input_.snapshot();
 report.quit_requested=input_snapshot.quit_requested;
 if(callbacks.fixed_update){
  for(std::size_t i=0;i<report.timing.fixed_steps;++i){
   callbacks.fixed_update(report.timing.fixed_delta_seconds,input_snapshot);
   ++report.update_callbacks;
  }
 }
 if(callbacks.render){
  callbacks.render(report.timing.interpolation_alpha,input_snapshot);
  report.rendered=true;
 }
 input_.end_frame();
 return report;
}
InputState& EngineLoop::input() noexcept{return input_;}
const InputState& EngineLoop::input() const noexcept{return input_;}
void EngineLoop::reset() noexcept{clock_.reset();input_.reset();}
}