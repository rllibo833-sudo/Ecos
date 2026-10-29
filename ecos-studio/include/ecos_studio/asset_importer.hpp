#pragma once

#include "ecos_studio/mesh.hpp"
#include "ecos_studio/material.hpp"
#include "ecos_studio/project.hpp"
#include "ecos_studio/result.hpp"

#include <filesystem>
#include <string>

namespace ecos::studio {

struct ImportedMaterialAsset {
    std::string id;
    std::filesystem::path relative_path;
    std::string content_sha256;
    std::size_t material_count{};
};

struct ImportedAsset {
    std::string id;
    std::filesystem::path relative_path;
    std::string content_sha256;
    std::size_t vertex_count{};
    std::size_t triangle_count{};
    MeshBounds bounds{};
};

// Validates and imports a Wavefront OBJ into an existing ECOS Studio project.
// The original bytes are copied into assets/imported and a versioned .ecasset sidecar is written.
[[nodiscard]] Result<ImportedAsset> import_obj_asset(
    const std::filesystem::path& project_root,
    const std::filesystem::path& source_path,
    const std::string& asset_id);

// Imports validated Wavefront MTL files into assets/imported with versioned metadata.
[[nodiscard]] Result<ImportedMaterialAsset> import_mtl_asset(
    const std::filesystem::path& project_root,
    const std::filesystem::path& source_path,
    const std::string& asset_id);

// Verifies imported-asset metadata, safe paths, content hashes, and parsed source statistics.
[[nodiscard]] ValidationResult validate_imported_assets(
    const std::filesystem::path& project_root);

} // namespace ecos::studio
