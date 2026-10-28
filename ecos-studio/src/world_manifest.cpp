#include "ecos_studio/world_manifest.hpp"
#include "ecos_studio/world.hpp"

#include <charconv>
#include <fstream>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace ecos::studio {
namespace {
std::vector<std::string> split(const std::string& value, char delimiter) {
    std::vector<std::string> parts;
    std::size_t start = 0;
    while (true) {
        const auto at = value.find(delimiter, start);
        parts.push_back(value.substr(start, at == std::string::npos ? at : at - start));
        if (at == std::string::npos) break;
        start = at + 1;
    }
    return parts;
}

bool parse_coordinate(const std::string& text, std::int64_t& value) {
    if (text.empty()) return false;
    const auto* begin = text.data();
    const auto* end = begin + text.size();
    const auto result = std::from_chars(begin, end, value);
    return result.ec == std::errc{} && result.ptr == end;
}

void add_error(ValidationResult& result, std::size_t line, const std::string& message) {
    result.ok = false;
    result.errors.push_back("line " + std::to_string(line) + ": " + message);
}
}

ValidationResult validate_world_manifest(const std::filesystem::path& path) {
    ValidationResult result;
    std::ifstream input(path);
    if (!input) {
        result.ok = false;
        result.errors.push_back("cannot open world manifest: " + path.string());
        return result;
    }

    bool version_seen = false;
    bool id_seen = false;
    std::string world_id;
    std::set<std::tuple<std::int64_t, std::int64_t, std::int64_t>> coordinates;
    WorldRuntime world;
    std::string line;
    std::size_t line_number = 0;
    std::size_t cell_count = 0;

    while (std::getline(input, line)) {
        ++line_number;
        if (line.empty() || line[0] == '#') continue;
        const auto separator = line.find('=');
        if (separator == std::string::npos) {
            add_error(result, line_number, "expected key=value");
            continue;
        }
        const auto key = line.substr(0, separator);
        const auto value = line.substr(separator + 1);
        if (key == "format_version") {
            if (version_seen) { add_error(result, line_number, "duplicate format_version"); continue; }
            version_seen = true;
            if (value != "1") add_error(result, line_number, "unsupported format_version (expected 1)");
        } else if (key == "world_id") {
            if (id_seen) { add_error(result, line_number, "duplicate world_id"); continue; }
            id_seen = true;
            world_id = value;
            if (world_id.empty()) add_error(result, line_number, "world_id must not be empty");
        } else if (key == "cell") {
            const auto fields = split(value, '|');
            if (fields.size() != 4) {
                add_error(result, line_number, "cell must be id|region_id|x,y,z|relative_content_path");
                continue;
            }
            const auto coords = split(fields[2], ',');
            if (coords.size() != 3) {
                add_error(result, line_number, "cell coordinates must contain x,y,z");
                continue;
            }
            CellCoord coord;
            if (!parse_coordinate(coords[0], coord.x) ||
                !parse_coordinate(coords[1], coord.y) ||
                !parse_coordinate(coords[2], coord.z)) {
                add_error(result, line_number, "cell coordinates must be signed 64-bit integers");
                continue;
            }
            const auto position = std::make_tuple(coord.x, coord.y, coord.z);
            if (!coordinates.insert(position).second) {
                add_error(result, line_number, "duplicate cell coordinates");
                continue;
            }
            auto registered = world.register_cell({fields[0], fields[1], coord, fields[3]});
            if (!registered) {
                add_error(result, line_number, registered.error().message);
                continue;
            }
            ++cell_count;
        } else {
            add_error(result, line_number, "unknown key '" + key + "'");
        }
    }

    if (!version_seen) add_error(result, 0, "missing format_version");
    if (!id_seen) add_error(result, 0, "missing world_id");
    if (cell_count == 0) add_error(result, 0, "world manifest must define at least one cell");
    if (input.bad()) {
        result.ok = false;
        result.errors.push_back("I/O error while reading world manifest");
    }
    if (result.ok) result.warnings.push_back("Validated world '" + world_id + "' with " +
        std::to_string(cell_count) + " cell(s).");
    return result;
}

 
Result<WorldManifest> load_world_manifest(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        return Result<WorldManifest>::failure({ErrorCode::io_error,
            "Cannot open world manifest: " + path.string()});
    }

    WorldManifest manifest;
    manifest.source_path = path;
    bool version_seen = false;
    bool id_seen = false;
    std::set<std::tuple<std::int64_t, std::int64_t, std::int64_t>> coordinates;
    std::set<std::string> cell_ids;
    std::string line;
    std::size_t line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        if (line.empty() || line[0] == '#') continue;
        const auto separator = line.find('=');
        if (separator == std::string::npos) {
            return Result<WorldManifest>::failure({ErrorCode::invalid_argument,
                "World manifest line " + std::to_string(line_number) + " must be key=value"});
        }
        const auto key = line.substr(0, separator);
        const auto value = line.substr(separator + 1);
        if (key == "format_version") {
            if (version_seen) return Result<WorldManifest>::failure(
                {ErrorCode::invalid_argument, "Duplicate world format_version"});
            version_seen = true;
            if (value != "1") return Result<WorldManifest>::failure(
                {ErrorCode::unsupported, "Unsupported world manifest format version"});
        } else if (key == "world_id") {
            if (id_seen) return Result<WorldManifest>::failure(
                {ErrorCode::invalid_argument, "Duplicate world_id"});
            id_seen = true;
            if (value.empty() || value.find_first_of("|\r\n") != std::string::npos)
                return Result<WorldManifest>::failure(
                    {ErrorCode::invalid_argument, "Invalid world_id"});
            manifest.world_id = value;
        } else if (key == "cell") {
            const auto fields = split(value, '|');
            if (fields.size() != 4 || fields[0].empty() || fields[1].empty() || fields[3].empty()) {
                return Result<WorldManifest>::failure({ErrorCode::invalid_argument,
                    "World manifest line " + std::to_string(line_number) +
                    " must be id|region_id|x,y,z|relative_content_path"});
            }
            const auto coords = split(fields[2], ',');
            CellCoord coord;
            if (coords.size() != 3 || !parse_coordinate(coords[0], coord.x) ||
                !parse_coordinate(coords[1], coord.y) || !parse_coordinate(coords[2], coord.z)) {
                return Result<WorldManifest>::failure({ErrorCode::invalid_argument,
                    "Invalid cell coordinates at world manifest line " + std::to_string(line_number)});
            }
            if (!cell_ids.insert(fields[0]).second) {
                return Result<WorldManifest>::failure({ErrorCode::invalid_argument,
                    "Duplicate world cell id: " + fields[0]});
            }
            if (!coordinates.emplace(coord.x, coord.y, coord.z).second) {
                return Result<WorldManifest>::failure({ErrorCode::invalid_argument,
                    "Duplicate world cell coordinates at line " + std::to_string(line_number)});
            }
            WorldRuntime path_validator;
            auto registered = path_validator.register_cell({fields[0], fields[1], coord, fields[3]});
            if (!registered) return Result<WorldManifest>::failure(registered.error());
            manifest.cells.push_back({fields[0], fields[1], coord, fields[3]});
        } else {
            return Result<WorldManifest>::failure({ErrorCode::invalid_argument,
                "Unknown world manifest key '" + key + "'"});
        }
    }
    if (input.bad()) return Result<WorldManifest>::failure(
        {ErrorCode::io_error, "I/O error while reading world manifest"});
    if (!version_seen || !id_seen || manifest.cells.empty()) {
        return Result<WorldManifest>::failure({ErrorCode::invalid_argument,
            "World manifest requires format_version=1, world_id, and at least one cell"});
    }
    return Result<WorldManifest>::success(std::move(manifest));
}

Result<WorldRuntime> WorldManifest::create_runtime(std::size_t max_loaded_cells) const {
    if (cells.empty() || world_id.empty()) {
        return Result<WorldRuntime>::failure({ErrorCode::invalid_state,
            "Cannot instantiate an empty or unnamed world manifest"});
    }
    WorldRuntime runtime(max_loaded_cells);
    for (const auto& cell : cells) {
        auto registered = runtime.register_cell(cell);
        if (!registered) return Result<WorldRuntime>::failure(registered.error());
    }
    return Result<WorldRuntime>::success(std::move(runtime));
}
\n} // namespace ecos::studio
