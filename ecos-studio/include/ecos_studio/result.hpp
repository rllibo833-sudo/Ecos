#pragma once

#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace ecos::studio {

enum class ErrorCode {
    none,
    invalid_argument,
    invalid_state,
    not_found,
    permission_denied,
    io_error,
    path_escape,
    unsupported,
    internal_error
};

struct Error {
    ErrorCode code{ErrorCode::none};
    std::string message;

    [[nodiscard]] explicit operator bool() const noexcept {
        return code != ErrorCode::none;
    }
};

template <typename T>
class Result {
public:
    static Result success(T value) { return Result(std::move(value)); }
    static Result failure(Error error) { return Result(std::move(error)); }

    [[nodiscard]] bool has_value() const noexcept {
        return std::holds_alternative<T>(storage_);
    }
    [[nodiscard]] explicit operator bool() const noexcept { return has_value(); }

    T& value() & { return std::get<T>(storage_); }
    const T& value() const& { return std::get<T>(storage_); }
    T&& value() && { return std::get<T>(std::move(storage_)); }
    const Error& error() const& { return std::get<Error>(storage_); }

private:
    explicit Result(T value) : storage_(std::move(value)) {}
    explicit Result(Error error) : storage_(std::move(error)) {}
    std::variant<T, Error> storage_;
};

template <>
class Result<void> {
public:
    static Result success() { return Result(); }
    static Result failure(Error error) { return Result(std::move(error)); }

    [[nodiscard]] bool has_value() const noexcept { return !error_; }
    [[nodiscard]] explicit operator bool() const noexcept { return has_value(); }
    [[nodiscard]] const Error& error() const noexcept { return error_; }

private:
    Result() = default;
    explicit Result(Error error) : error_(std::move(error)) {}
    Error error_{};
};

[[nodiscard]] const char* error_code_name(ErrorCode code) noexcept;

} // namespace ecos::studio
