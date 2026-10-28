#pragma once
#include <cstddef>
namespace ecos::studio {
struct FrameTiming {
 double frame_delta_seconds{0.0};
 double fixed_delta_seconds{1.0/60.0};
 std::size_t fixed_steps{0};
 double interpolation_alpha{0.0};
 double dropped_time_seconds{0.0};
};
class FrameClock {
public:
 explicit FrameClock(double fixed_delta_seconds=1.0/60.0,
                     double max_frame_delta_seconds=0.25,
                     std::size_t max_fixed_steps=8) noexcept;
 [[nodiscard]] FrameTiming advance(double elapsed_seconds) noexcept;
 void reset() noexcept;
 [[nodiscard]] double fixed_delta_seconds() const noexcept;
 [[nodiscard]] double accumulator_seconds() const noexcept;
private:
 double fixed_delta_seconds_;
 double max_frame_delta_seconds_;
 std::size_t max_fixed_steps_;
 double accumulator_seconds_{0.0};
};
}