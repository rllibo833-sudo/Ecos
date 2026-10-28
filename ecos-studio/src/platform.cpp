#include "ecos_studio/platform.hpp"

#include <fstream>
#include <sstream>
#include <system_error>

namespace ecos::studio::platform {

std::uint64_t monotonic_time_ms() noexcept {
    const auto elapsed = MonotonicClock::now().time_since_epoch();
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
}

std::string operating_system_name() {
#if defined(_WIN32)
    return "Windows";
#elif defined(__APPLE__)
    return "Apple platforms";
#elif defined(__linux__)
    return "Linux";
#elif defined(__FreeBSD__)
    return "FreeBSD";
#else
    return "Unknown";
#endif
}

Result<std::filesystem::path> safe_relative_path(const std::filesystem::path& path) {
    if (path.empty()) {
        return Result<std::filesystem::path>::failure(
            {ErrorCode::invalid_argument, "Path must not be empty"});
    }
    if (path.is_absolute() || path.has_root_name() || path.has_root_directory()) {
        return Result<std::filesystem::path>::failure(
            {ErrorCode::path_escape, "Absolute paths are not allowed here"});
    }

    const auto normalized = path.lexically_normal();
    for (const auto& component : normalized) {
        if (component == "..") {
            return Result<std::filesystem::path>::failure(
                {ErrorCode::path_escape, "Path must not escape its project root"});
        }
    }
    if (normalized == "." || normalized.empty()) {
        return Result<std::filesystem::path>::failure(
            {ErrorCode::invalid_argument, "Path must identify a project-relative target"});
    }
    return Result<std::filesystem::path>::success(normalized);
}

Result<void> ensure_directory(const std::filesystem::path& path) {
    if (path.empty()) {
        return Result<void>::failure({ErrorCode::invalid_argument, "Directory path is empty"});
    }

    std::error_code ec;
    std::filesystem::create_directories(path, ec);
    if (ec) {
        return Result<void>::failure({ErrorCode::io_error, "Cannot create directory: " + ec.message()});
    }
    if (!std::filesystem::is_directory(path, ec) || ec) {
        return Result<void>::failure({ErrorCode::io_error, "Target exists but is not a directory"});
    }
    return Result<void>::success();
}

Result<std::string> read_text_file(const std::filesystem::path& path) {
    if (path.empty()) {
        return Result<std::string>::failure({ErrorCode::invalid_argument, "File path is empty"});
    }
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return Result<std::string>::failure({ErrorCode::io_error, "Cannot open file: " + path.string()});
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    if (input.bad()) {
        return Result<std::string>::failure({ErrorCode::io_error, "Failed while reading file: " + path.string()});
    }
    return Result<std::string>::success(buffer.str());
}

} // namespace ecos::studio::platform
