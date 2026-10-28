#pragma once
#include "ecos_studio/scene.hpp"
#include "ecos_studio/world_manifest.hpp"
#include <cstddef>
#include <filesystem>
#include <string>
namespace ecos::studio {
struct WorldCellContent {
    std::string cell_id;
    std::filesystem::path source_path;
    SceneGraph scene;
};
[[nodiscard]] Result<WorldCellContent> load_world_cell_content(
    const WorldManifest& manifest, const WorldCell& cell,
    std::size_t max_bytes = 16U * 1024U * 1024U);
} // namespace ecos::studio
