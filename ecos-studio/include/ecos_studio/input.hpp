#pragma once
#include "ecos_studio/result.hpp"
#include <array>
#include <cstdint>
namespace ecos::studio {
enum class InputKey : std::uint16_t { Unknown=0, W, A, S, D, Q, E, Space, Escape, Left, Right, Up, Down, Count };
enum class InputAction : std::uint8_t { Release=0, Press=1, Repeat=2 };
struct InputSnapshot {
 std::array<bool,static_cast<std::size_t>(InputKey::Count)> keys{};
 std::array<bool,8> mouse_buttons{};
 double mouse_x{0},mouse_y{0},mouse_delta_x{0},mouse_delta_y{0},scroll_x{0},scroll_y{0};
 bool quit_requested{false};
 [[nodiscard]] bool down(InputKey key) const noexcept;
};
class InputState {
public:
 [[nodiscard]] Result<void> key_event(InputKey key,InputAction action);
 [[nodiscard]] Result<void> mouse_button_event(std::uint8_t button,InputAction action);
 void mouse_move(double x,double y) noexcept;
 void scroll(double x,double y) noexcept;
 void request_quit() noexcept;
 [[nodiscard]] InputSnapshot snapshot() const noexcept;
 void end_frame() noexcept;
 void reset() noexcept;
private: InputSnapshot state_{}; bool have_mouse_position_{false};
};
}