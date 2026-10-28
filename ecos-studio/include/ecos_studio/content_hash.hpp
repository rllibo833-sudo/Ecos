#pragma once

#include <string>
#include <string_view>

namespace ecos::studio {

// Returns the lowercase hexadecimal SHA-256 digest of the exact input bytes.
[[nodiscard]] std::string sha256_hex(std::string_view data);

} // namespace ecos::studio
