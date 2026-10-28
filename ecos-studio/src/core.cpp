#include "ecos_studio/core.hpp"

#include <ostream>

namespace ecos::studio {

Logger::Logger(std::ostream& output) : output_(output) {}

void Logger::write(LogLevel level, std::string_view message) {
    std::lock_guard<std::mutex> lock(mutex_);
    output_ << "[ECOS][" << log_level_name(level) << "] " << message << '\n';
}

bool Runtime::initialize() {
    if (state_ != RuntimeState::cold) return false;
    state_ = RuntimeState::ready;
    return true;
}

void Runtime::shutdown() noexcept {
    if (state_ == RuntimeState::ready) state_ = RuntimeState::stopped;
}

RuntimeState Runtime::state() const noexcept { return state_; }
bool Runtime::is_ready() const noexcept { return state_ == RuntimeState::ready; }

std::string_view runtime_state_name(RuntimeState state) noexcept {
    switch (state) {
        case RuntimeState::cold: return "cold";
        case RuntimeState::ready: return "ready";
        case RuntimeState::stopped: return "stopped";
    }
    return "unknown";
}

std::string_view log_level_name(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::debug: return "DEBUG";
        case LogLevel::info: return "INFO";
        case LogLevel::warning: return "WARNING";
        case LogLevel::error: return "ERROR";
    }
    return "UNKNOWN";
}

} // namespace ecos::studio
