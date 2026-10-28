#include "ecos_studio/result.hpp"

namespace ecos::studio {

const char* error_code_name(ErrorCode code) noexcept {
    switch (code) {
    case ErrorCode::none: return "none";
    case ErrorCode::invalid_argument: return "invalid_argument";
    case ErrorCode::invalid_state: return "invalid_state";
    case ErrorCode::not_found: return "not_found";
    case ErrorCode::permission_denied: return "permission_denied";
    case ErrorCode::io_error: return "io_error";
    case ErrorCode::path_escape: return "path_escape";
    case ErrorCode::unsupported: return "unsupported";
    case ErrorCode::internal_error: return "internal_error";
    }
    return "unknown";
}

} // namespace ecos::studio
