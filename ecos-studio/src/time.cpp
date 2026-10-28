#include "ecos_studio/time.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
namespace ecos::studio {
FrameClock::FrameClock(double fixed_delta_seconds,double max_frame_delta_seconds,std::size_t max_fixed_steps) noexcept
 : fixed_delta_seconds_(std::isfinite(fixed_delta_seconds)&&fixed_delta_seconds>0.0?fixed_delta_seconds:1.0/60.0),
   max_frame_delta_seconds_(std::isfinite(max_frame_delta_seconds)&&max_frame_delta_seconds>0.0?max_frame_delta_seconds:0.25),
   max_fixed_steps_(max_fixed_steps==0?1:max_fixed_steps) {}
FrameTiming FrameClock::advance(double elapsed_seconds) noexcept {
 FrameTiming result;
 result.fixed_delta_seconds=fixed_delta_seconds_;
 if(!std::isfinite(elapsed_seconds)||elapsed_seconds<0.0)elapsed_seconds=0.0;
 result.frame_delta_seconds=std::min(elapsed_seconds,max_frame_delta_seconds_);
 result.dropped_time_seconds=elapsed_seconds-result.frame_delta_seconds;
 if(accumulator_seconds_>std::numeric_limits<double>::max()-result.frame_delta_seconds){
  result.dropped_time_seconds+=accumulator_seconds_;
  accumulator_seconds_=0.0;
 }else accumulator_seconds_+=result.frame_delta_seconds;
 while(accumulator_seconds_>=fixed_delta_seconds_&&result.fixed_steps<max_fixed_steps_){
  accumulator_seconds_-=fixed_delta_seconds_;
  ++result.fixed_steps;
 }
 if(accumulator_seconds_>=fixed_delta_seconds_){
  const double retained=std::fmod(accumulator_seconds_,fixed_delta_seconds_);
  result.dropped_time_seconds+=accumulator_seconds_-retained;
  accumulator_seconds_=retained;
 }
 result.interpolation_alpha=std::clamp(accumulator_seconds_/fixed_delta_seconds_,0.0,1.0);
 return result;
}
void FrameClock::reset() noexcept {accumulator_seconds_=0.0;}
double FrameClock::fixed_delta_seconds() const noexcept{return fixed_delta_seconds_;}
double FrameClock::accumulator_seconds() const noexcept{return accumulator_seconds_;}
}