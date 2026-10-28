#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace ecos::studio {

struct ModResolution {
    bool ok{true};
    std::vector<std::string> load_order;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

// Scans immediate child directories of mods_root for valid data-only mod packages.
// Dependencies are resolved only against packages already present on disk; no code is executed.
ModResolution resolve_mod_dependencies(const std::filesystem::path& mods_root);

} // namespace ecos::studio
