#pragma once

#include "ecos_studio/project.hpp"

#include <filesystem>

namespace ecos::studio {

[[nodiscard]] ValidationResult validate_world_manifest(const std::filesystem::path& path);

} // namespace ecos::studio
