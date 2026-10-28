#pragma once

#include "ecos_studio/project.hpp"
#include "ecos_studio/world.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace ecos::studio {

// Parsed, validated world data ready to be instantiated by a runtime.
struct WorldManifest {
    std::string world_id;
    std::filesystem::path source_path;
    std::vector<WorldCell> cells;

    [[nodiscard]] Result<WorldRuntime> create_runtime(std::size_t max_loaded_cells = 64) const;
};

// Loads and validates the manifest instead of merely checking its syntax.
// Cell content paths remain safe paths relative to the project/world root.
[[nodiscard]] Result<WorldManifest> load_world_manifest(const std::filesystem::path& path);
[[nodiscard]] ValidationResult validate_world_manifest(const std::filesystem::path& path);

} // namespace ecos::studio
