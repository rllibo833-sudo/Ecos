#include "ecos_studio/asset_importer.hpp"

#include "ecos_studio/content_hash.hpp"
#include "ecos_studio/material.hpp"
#include "ecos_studio/platform.hpp"
#include "ecos_studio/project.hpp"

#include <charconv>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
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

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

bool parse_metadata(const std::string& contents, std::map<std::string, std::string>& fields,
                    std::string& error) {
    std::istringstream input(contents);
    std::string line;
    std::size_t line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        line = trim(std::move(line));
        if (line.empty() || line.front() == '#') continue;
        const auto separator = line.find('=');
        if (separator == std::string::npos) {
            error = "Expected key=value at metadata line " + std::to_string(line_number);
            return false;
        }
        const auto key = trim(line.substr(0, separator));
        const auto value = trim(line.substr(separator + 1));
        if (key.empty() || value.empty()) {
            error = "Empty key or value at metadata line " + std::to_string(line_number);
            return false;
        }
        if (!fields.emplace(key, value).second) {
            error = "Duplicate asset metadata field: " + key;
            return false;
        }
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

bool read_required(const std::map<std::string, std::string>& fields, const std::string& key,
                   std::string& value, std::string& error) {
    const auto found = fields.find(key);
    if (found == fields.end() || found->second.empty()) {
        error = "Missing required asset metadata field: " + key;
        return false;
    }
    value = found->second;
    return true;
}

bool parse_count(const std::string& text, std::size_t& value) {
    if (text.empty()) return false;
    std::size_t parsed{};
    const auto result = std::from_chars(text.data(), text.data() + text.size(), parsed);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) return false;
    value = parsed;
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

Result<ImportedMaterialAsset> import_mtl_asset(const fs::path& project_root,
                                         const fs::path& source_path,
                                         const std::string& asset_id) {
    if (!valid_asset_id(asset_id)) {
        return Result<ImportedMaterialAsset>::failure({ErrorCode::invalid_argument,
            "Asset ID must start with a letter or digit and contain only lowercase letters, digits, '.', '-' or '_'."});
    }
    const auto project = validate_project(project_root);
    if (!project.ok) {
        const std::string reason = project.errors.empty() ? "Project validation failed" : project.errors.front();
        return Result<ImportedMaterialAsset>::failure({ErrorCode::invalid_state,
            "Cannot import into an invalid ECOS Studio project: " + reason});
    }
    std::error_code ec;
    if (!fs::is_regular_file(source_path, ec) || ec) {
        return Result<ImportedMaterialAsset>::failure({ErrorCode::not_found,
            "MTL source is not a readable regular file: " + source_path.string()});
    }
    auto source = platform::read_text_file(source_path);
    if (!source) return Result<ImportedMaterialAsset>::failure(source.error());
    auto materials = import_mtl_materials(source.value());
    if (!materials) return Result<ImportedMaterialAsset>::failure(materials.error());
    const auto canonical_root = fs::canonical(project_root, ec);
    if (ec) return Result<ImportedMaterialAsset>::failure({ErrorCode::io_error,
        "Cannot resolve project root: " + ec.message()});
    const fs::path relative_path = fs::path("assets") / "imported" / (asset_id + ".mtl");
    const fs::path assets_dir = canonical_root / "assets";
    const fs::path import_dir = assets_dir / "imported";
    const fs::path destination = import_dir / (asset_id + ".mtl");
    const fs::path sidecar = import_dir / (asset_id + ".ecasset");
    std::string directory_error;
    if (!reject_symlink_or_non_directory(assets_dir, directory_error) ||
        !reject_symlink_or_non_directory(import_dir, directory_error)) {
        return Result<ImportedMaterialAsset>::failure({ErrorCode::path_escape, directory_error});
    }
    auto directory = platform::ensure_directory(import_dir);
    if (!directory) return Result<ImportedMaterialAsset>::failure(directory.error());
    if (!reject_symlink_or_non_directory(assets_dir, directory_error) ||
        !reject_symlink_or_non_directory(import_dir, directory_error)) {
        return Result<ImportedMaterialAsset>::failure({ErrorCode::path_escape, directory_error});
    }
    const auto digest = sha256_hex(source.value());
    std::ostringstream metadata;
    metadata << "# ECOS Studio imported asset metadata v1\n"
             << "format_version=1\n"
             << "id=" << asset_id << "\n"
             << "type=material.mtl\n"
             << "source=" << relative_path.generic_string() << "\n"
             << "content_sha256=" << digest << "\n"
             << "material_count=" << materials.value().size() << "\n";
    std::string write_error;
    if (!write_new_file(destination, source.value(), write_error)) {
        return Result<ImportedMaterialAsset>::failure({ErrorCode::io_error, write_error});
    }
    if (!write_new_file(sidecar, metadata.str(), write_error)) {
        fs::remove(destination, ec);
        return Result<ImportedMaterialAsset>::failure({ErrorCode::io_error, write_error});
    }
    return Result<ImportedMaterialAsset>::success(
        ImportedMaterialAsset{asset_id, relative_path, digest, materials.value().size()});
}

ValidationResult validate_imported_assets(const fs::path& project_root) {
    ValidationResult result;
    const auto project = validate_project(project_root);
    if (!project.ok) {
        result.ok = false;
        result.errors.push_back("Invalid ECOS Studio project: " +
            (project.errors.empty() ? std::string("unknown validation error") : project.errors.front()));
        return result;
    }

    std::error_code ec;
    const auto canonical_root = fs::canonical(project_root, ec);
    if (ec) {
        result.ok = false;
        result.errors.push_back("Cannot resolve project root: " + ec.message());
        return result;
    }
    const fs::path assets_dir = canonical_root / "assets";
    const fs::path import_dir = assets_dir / "imported";
    std::string directory_error;
    if (!reject_symlink_or_non_directory(assets_dir, directory_error) ||
        !reject_symlink_or_non_directory(import_dir, directory_error)) {
        result.ok = false;
        result.errors.push_back(directory_error);
        return result;
    }
    if (!fs::exists(import_dir, ec)) {
        if (ec) {
            result.ok = false;
            result.errors.push_back("Cannot inspect imported asset directory: " + ec.message());
        }
        return result;
    }

    std::map<std::string, fs::path> ids;
    fs::directory_iterator iterator(import_dir, ec), end;
    if (ec) {
        result.ok = false;
        result.errors.push_back("Cannot scan imported assets: " + ec.message());
        return result;
    }
    for (; iterator != end; iterator.increment(ec)) {
        if (ec) {
            result.ok = false;
            result.errors.push_back("Failed while scanning imported assets: " + ec.message());
            break;
        }
        const auto status = iterator->symlink_status(ec);
        if (ec) {
            result.ok = false;
            result.errors.push_back("Cannot inspect imported asset entry: " + ec.message());
            ec.clear();
            continue;
        }
        if (fs::is_symlink(status)) {
            result.ok = false;
            result.errors.push_back("Symlinks are not allowed in assets/imported: " + iterator->path().filename().string());
            continue;
        }
        if (!fs::is_regular_file(status) || iterator->path().extension() != ".ecasset") continue;

        const auto sidecar_path = iterator->path();
        const auto sidecar_text = platform::read_text_file(sidecar_path);
        if (!sidecar_text) {
            result.ok = false;
            result.errors.push_back(sidecar_text.error().message);
            continue;
        }
        std::map<std::string, std::string> fields;
        std::string error;
        if (!parse_metadata(sidecar_text.value(), fields, error)) {
            result.ok = false;
            result.errors.push_back(sidecar_path.filename().string() + ": " + error);
            continue;
        }
        std::string id, type, source_path, digest, version;
        bool fields_ok = true;
        for (const auto* key : {"id", "type", "source", "content_sha256", "format_version"}) {
            std::string ignored;
            if (!read_required(fields, key, ignored, error)) {
                result.ok = false; result.errors.push_back(sidecar_path.filename().string() + ": " + error);
                fields_ok = false;
            }
        }
        if (!fields_ok) continue;
        id = fields["id"]; type = fields["type"]; source_path = fields["source"];
        digest = fields["content_sha256"]; version = fields["format_version"];
        if (!valid_asset_id(id) || id != sidecar_path.stem().string()) {
            result.ok = false; result.errors.push_back(sidecar_path.filename().string() + ": invalid or mismatched asset ID"); continue;
        }
        if (!ids.emplace(id, sidecar_path).second) {
            result.ok = false; result.errors.push_back("Duplicate imported asset ID: " + id); continue;
        }
        if (version != "1" || (type != "mesh.obj" && type != "material.mtl")) {
            result.ok = false; result.errors.push_back(id + ": unsupported asset metadata version or type"); continue;
        }
        const std::string extension = type == "mesh.obj" ? ".obj" : ".mtl";
        const fs::path expected_relative = fs::path("assets") / "imported" / (id + extension);
        auto safe = platform::safe_relative_path(fs::path(source_path));
        if (!safe || safe.value() != expected_relative) {
            result.ok = false; result.errors.push_back(id + ": source path must match its expected project-relative asset path"); continue;
        }
        const auto source_file = canonical_root / safe.value();
        const auto source_status = fs::symlink_status(source_file, ec);
        if (ec || fs::is_symlink(source_status) || !fs::is_regular_file(source_status)) {
            result.ok = false; result.errors.push_back(id + ": source asset is missing or not a regular non-symlink file"); ec.clear(); continue;
        }
        const auto source_text = platform::read_text_file(source_file);
        if (!source_text) { result.ok = false; result.errors.push_back(id + ": " + source_text.error().message); continue; }
        if (sha256_hex(source_text.value()) != digest) {
            result.ok = false; result.errors.push_back(id + ": content SHA-256 does not match metadata"); continue;
        }
        if (type == "mesh.obj") {
            std::string vertex_text, triangle_text, bounds_min, bounds_max;
            bool mesh_fields_ok = true;
            for (const auto* key : {"vertex_count", "triangle_count", "bounds_min", "bounds_max"}) {
                std::string ignored;
                if (!read_required(fields, key, ignored, error)) {
                    result.ok = false; result.errors.push_back(id + ": " + error); mesh_fields_ok = false;
                }
            }
            if (!mesh_fields_ok) continue;
            vertex_text = fields["vertex_count"]; triangle_text = fields["triangle_count"];
            bounds_min = fields["bounds_min"]; bounds_max = fields["bounds_max"];
            auto mesh = import_obj_mesh(source_text.value());
            if (!mesh) { result.ok = false; result.errors.push_back(id + ": imported OBJ is no longer valid: " + mesh.error().message); continue; }
            std::size_t vertices{}, triangles{};
            if (!parse_count(vertex_text, vertices) || !parse_count(triangle_text, triangles) ||
                vertices != mesh.value().positions.size() || triangles != mesh.value().indices.size() / 3U ||
                bounds_min != vector_text(mesh.value().bounds.minimum) || bounds_max != vector_text(mesh.value().bounds.maximum)) {
                result.ok = false; result.errors.push_back(id + ": stored mesh statistics do not match the source geometry");
            }
        } else {
            std::string count_text;
            if (!read_required(fields, "material_count", count_text, error)) {
                result.ok = false; result.errors.push_back(id + ": " + error); continue;
            }
            auto materials = import_mtl_materials(source_text.value());
            std::size_t count{};
            if (!materials || !parse_count(count_text, count) || count != materials.value().size()) {
                result.ok = false; result.errors.push_back(id + ": stored material count does not match valid MTL source");
            }
        }
    }
    return result;
}

} // namespace ecos::studio
