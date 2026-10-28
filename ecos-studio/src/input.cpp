#include "ecos_studio/input.hpp"
namespace ecos::studio {
void InputEventQueue::push(InputEvent event) {
    std::lock_guard lock(mutex_);
    events_.push_back(event);
}
std::vector<InputEvent> InputEventQueue::drain() {
    std::lock_guard lock(mutex_);
    std::vector<InputEvent> result;
    result.swap(events_);
    return result;
}
std::size_t InputEventQueue::size() const {
    std::lock_guard lock(mutex_);
    return events_.size();
}
void InputState::begin_frame() noexcept {
    keys_pressed_.clear(); keys_released_.clear();
    buttons_pressed_.clear(); buttons_released_.clear();
    wheel_x_ = 0.0F; wheel_y_ = 0.0F;
}
void InputState::apply(const InputEvent& event) {
    switch (event.type) {
    case InputEventType::key_pressed:
        if (keys_down_.insert(event.code).second) keys_pressed_.insert(event.code);
        break;
    case InputEventType::key_released:
        if (keys_down_.erase(event.code) != 0) keys_released_.insert(event.code);
        break;
    case InputEventType::mouse_button_pressed:
        if (buttons_down_.insert(event.code).second) buttons_pressed_.insert(event.code);
        break;
    case InputEventType::mouse_button_released:
        if (buttons_down_.erase(event.code) != 0) buttons_released_.insert(event.code);
        break;
    case InputEventType::mouse_moved:
        mouse_x_ = event.x; mouse_y_ = event.y;
        break;
    case InputEventType::mouse_wheel:
        wheel_x_ += event.x; wheel_y_ += event.y;
        break;
    case InputEventType::focus_lost:
        for (const int code : keys_down_) keys_released_.insert(code);
        for (const int code : buttons_down_) buttons_released_.insert(code);
        keys_down_.clear(); buttons_down_.clear();
        break;
    }
}
bool InputState::key_down(int code) const noexcept { return keys_down_.contains(code); }
bool InputState::key_pressed(int code) const noexcept { return keys_pressed_.contains(code); }
bool InputState::key_released(int code) const noexcept { return keys_released_.contains(code); }
bool InputState::mouse_button_down(int code) const noexcept { return buttons_down_.contains(code); }
bool InputState::mouse_button_pressed(int code) const noexcept { return buttons_pressed_.contains(code); }
bool InputState::mouse_button_released(int code) const noexcept { return buttons_released_.contains(code); }
float InputState::mouse_x() const noexcept { return mouse_x_; }
float InputState::mouse_y() const noexcept { return mouse_y_; }
float InputState::wheel_x() const noexcept { return wheel_x_; }
float InputState::wheel_y() const noexcept { return wheel_y_; }
} // namespace ecos::studio
