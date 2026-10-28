#pragma once

#include <cstddef>
#include <mutex>
#include <unordered_set>
#include <vector>

namespace ecos::studio {
enum class InputEventType { key_pressed, key_released, mouse_button_pressed, mouse_button_released, mouse_moved, mouse_wheel, focus_lost };
struct InputEvent {
    InputEventType type{InputEventType::key_pressed};
    int code{0};
    float x{0.0F};
    float y{0.0F};
};
class InputEventQueue {
public:
    void push(InputEvent event);
    [[nodiscard]] std::vector<InputEvent> drain();
    [[nodiscard]] std::size_t size() const;
private:
    mutable std::mutex mutex_;
    std::vector<InputEvent> events_;
};
class InputState {
public:
    void begin_frame() noexcept;
    void apply(const InputEvent& event);
    [[nodiscard]] bool key_down(int code) const noexcept;
    [[nodiscard]] bool key_pressed(int code) const noexcept;
    [[nodiscard]] bool key_released(int code) const noexcept;
    [[nodiscard]] bool mouse_button_down(int code) const noexcept;
    [[nodiscard]] bool mouse_button_pressed(int code) const noexcept;
    [[nodiscard]] bool mouse_button_released(int code) const noexcept;
    [[nodiscard]] float mouse_x() const noexcept;
    [[nodiscard]] float mouse_y() const noexcept;
    [[nodiscard]] float wheel_x() const noexcept;
    [[nodiscard]] float wheel_y() const noexcept;
private:
    std::unordered_set<int> keys_down_, keys_pressed_, keys_released_;
    std::unordered_set<int> buttons_down_, buttons_pressed_, buttons_released_;
    float mouse_x_{0.0F}, mouse_y_{0.0F}, wheel_x_{0.0F}, wheel_y_{0.0F};
};
} // namespace ecos::studio
