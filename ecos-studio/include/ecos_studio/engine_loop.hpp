#pragma once
#include "ecos_studio/input.hpp"
#include "ecos_studio/time.hpp"
#include <functional>
namespace ecos::studio {
struct FrameCallbacks {
 std::function<void(double,const InputSnapshot&)> fixed_update;
 std::function<void(double,const InputSnapshot&)> render;
};
struct FrameReport {
 FrameTiming timing{};
 std::size_t update_callbacks{0};
 bool rendered{false};
 bool quit_requested{false};
};
class EngineLoop {
public:
 explicit EngineLoop(double fixed_delta_seconds=1.0/60.0,
                     double max_frame_delta_seconds=0.25,
                     std::size_t max_fixed_steps=8) noexcept;
 [[nodiscard]] FrameReport frame(double elapsed_seconds,const FrameCallbacks& callbacks);
 [[nodiscard]] InputState& input() noexcept;
 [[nodiscard]] const InputState& input() const noexcept;
 void reset() noexcept;
private:
 FrameClock clock_;
 InputState input_;
};
}