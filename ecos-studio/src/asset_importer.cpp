#include "ecos_studio/asset_importer.hpp"

#include "ecos_studio/content_hash.hpp"
#include "ecos_studio/platform.hpp"
#include "ecos_studio/project.hpp"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <system_error>
#include <utility>

namespace ecos::studio {
namespace fs = std::filesystem;
namespace {

bool valid_asset_id(const std::string& id) {
    if (id.empty() || !std::isalnum(static_cast<unsigned char>(id.front()))) return false;
    for (const unsigned char c : id) {
        if (!(std::islower(c) || std::isdigit(c) || c == '.' || c == '-' || c == '_')) return false;
    }
    return true;
}

bool write_new_file(const fs::path& path, const std::string& contents, std::string& error) {
    std::error_code ec;
    const auto status = fs::symlink_status(path, ec);
    if (!ec && status.type() != fs::file_type::not_found) {
        error = "Refusing to overwrite existing filesystem entry: " + path.string();
        return false;
    }
    if (ec && ec != std::errc::no_such_file_or_directory) {
        error = "Cannot inspect destination '" + path.string() + "': " + ec.message();
        return false;
    }
    ec.clear();
    std::ofstream output(path, std::ios::binary | std::ios::out);
    if (!output) {
        error = "Cannot create file: " + path.string();
        return false;
    }
    output.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    output.flush();
    if (!output) {
        error = "Failed while writing file: " + path.string();
        output.close();
        fs::remove(path, ec);
        return false;
    }
    return true;
}

bool reject_symlink_or_non_directory(const fs::path& path, std::string& error) {
    std::error_code ec;
    const auto status = fs::symlink_status(path, ec);
    if (ec == std::errc::no_such_file_or_directory) return true;
    if (ec) {
        error = "Cannot inspect project asset directory '" + path.string() + "': " + ec.message();
        return false;
    }
    if (fs::is_symlink(status) || !fs::is_directory(status)) {
        error = "Project asset directory must not be a symlink or non-directory: " + path.string();
        return false;
    }
    return true;
}

std::string vector_text(const Vec3& value) {
    std::ostringstream out;
    out << std::setprecision(std::numeric_limits<float>::max_digits10)
        << value.x << ',' << value.y << ',' << value.z;
    return out.str();
}

} // namespace

Result<ImportedAsset> import_obj_asset(const fs::path& project_root,
                                       const fs::path& source_path,
                                       const std::string& asset_id) {
    if (!valid_asset_id(asset_id)) {
        return Result<ImportedAsset>::failure({ErrorCode::invalid_argument,
            "Asset ID must start with a letter or digit and contain only lowercase letters, digits, '.', '-' or '_'."});
    }

    const auto project = validate_project(project_root);
    if (!project.ok) {
        const std::string reason = project.errors.empty() ? "Project validation failed" : project.errors.front();
        return Result<ImportedAsset>::failure({ErrorCode::invalid_state,
            "Cannot import into an invalid ECOS Studio project: " + reason});
    }

    std::error_code ec;
    if (!fs::is_regular_file(source_path, ec) || ec) {
        return Result<ImportedAsset>::failure({ErrorCode::not_found,
            "OBJ source is not a readable regular file: " + source_path.string()});
    }
    auto source = platform::read_text_file(source_path);
    if (!source) return Result<ImportedAsset>::failure(source.error());

    auto mesh = import_obj_mesh(source.value());
    if (!mesh) return Result<ImportedAsset>::failure(mesh.error());

    const auto canonical_root = fs::canonical(project_root, ec);
    if (ec) return Result<ImportedAsset>::failure({ErrorCode::io_error,
        "Cannot resolve project root: " + ec.message()});

    const fs::path relative_path = fs::path("assets") / "imported" / (asset_id + ".obj");
    const fs::path assets_dir = canonical_root / "assets";
    const fs::path import_dir = assets_dir / "imported";
    const fs::path destination = import_dir / (asset_id + ".obj");
    const fs::path sidecar = import_dir / (asset_id + ".ecasset");

    std::string directory_error;
    if (!reject_symlink_or_non_directory(assets_dir, directory_error) ||
        !reject_symlink_or_non_directory(import_dir, directory_error)) {
        return Result<ImportedAsset>::failure({ErrorCode::path_escape, directory_error});
    }
    auto directory = platform::ensure_directory(import_dir);
    if (!directory) return Result<ImportedAsset>::failure(directory.error());
    if (!reject_symlink_or_non_directory(assets_dir, directory_error) ||
        !reject_symlink_or_non_directory(import_dir, directory_error)) {
        return Result<ImportedAsset>::failure({ErrorCode::path_escape, directory_error});
    }

    const auto digest = sha256_hex(source.value());
    std::ostringstream metadata;
    metadata << "# ECOS Studio imported asset metadata v1\n"
             << "format_version=1\n"
             << "id=" << asset_id << "\n"
             << "type=mesh.obj\n"
             << "source=" << relative_path.generic_string() << "\n"
             << "content_sha256=" << digest << "\n"
             << "vertex_count=" << mesh.value().positions.size() << "\n"
             << "triangle_count=" << mesh.value().indices.size() / 3U << "\n"
             << "bounds_min=" << vector_text(mesh.value().bounds.minimum) << "\n"
             << "bounds_max=" << vector_text(mesh.value().bounds.maximum) << "\n";

    std::string write_error;
    if (!write_new_file(destination, source.value(), write_error)) {
        return Result<ImportedAsset>::failure({ErrorCode::io_error, write_error});
    }
    if (!write_new_file(sidecar, metadata.str(), write_error)) {
        fs::remove(destination, ec);
        return Result<ImportedAsset>::failure({ErrorCode::io_error, write_error});
    }

    ImportedAsset result;
    result.id = asset_id;
    result.relative_path = relative_path;
    result.content_sha256 = digest;
    result.vertex_count = mesh.value().positions.size();
    result.triangle_count = mesh.value().indices.size() / 3U;
    result.bounds = mesh.value().bounds;
    return Result<ImportedAsset>::success(std::move(result));
}

} // namespace ecos::studio
