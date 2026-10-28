#pragma once
#include "ecos_studio/result.hpp"
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <string>

namespace ecos::studio::platform {
using MonotonicClock = std::chrono::steady_clock;
[[nodiscard]] std::uint64_t monotonic_time_ms() noexcept;
[[nodiscard]] std::string operating_system_name();
[[nodiscard]] Result<std::filesystem::path> safe_relative_path(const std::filesystem::path& path);
[[nodiscard]] Result<void> ensure_directory(const std::filesystem::path& path);
[[nodiscard]] Result<std::string> read_text_file(const std::filesystem::path& path);
} // namespace ecos::studio::platform
