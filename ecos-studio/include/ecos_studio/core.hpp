#pragma once

#include <iosfwd>
#include <mutex>
#include <string_view>

namespace ecos::studio {

enum class LogLevel { debug, info, warning, error };

class Logger {
public:
    explicit Logger(std::ostream& output);
    void write(LogLevel level, std::string_view message);
private:
    std::ostream& output_;
    std::mutex mutex_;
};

enum class RuntimeState { cold, ready, stopped };

class Runtime {
public:
    bool initialize();
    void shutdown() noexcept;
    [[nodiscard]] RuntimeState state() const noexcept;
    [[nodiscard]] bool is_ready() const noexcept;
private:
    RuntimeState state_{RuntimeState::cold};
};

[[nodiscard]] std::string_view runtime_state_name(RuntimeState state) noexcept;
[[nodiscard]] std::string_view log_level_name(LogLevel level) noexcept;

} // namespace ecos::studio
