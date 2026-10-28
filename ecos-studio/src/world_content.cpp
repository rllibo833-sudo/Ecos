#include "ecos_studio/world_content.hpp"
#include "ecos_studio/platform.hpp"
#include <algorithm>
#include <fstream>
#include <limits>
#include <system_error>
#include <utility>
namespace ecos::studio {
namespace {
bool path_is_within(const std::filesystem::path& root, const std::filesystem::path& target) {
    auto a = root.begin(); auto b = target.begin();
    for (; a != root.end(); ++a, ++b) if (b == target.end() || *a != *b) return false;
    return true;
}
}
Result<WorldCellContent> load_world_cell_content(
    const WorldManifest& manifest, const WorldCell& cell, std::size_t max_bytes) {
    if (manifest.source_path.empty() || manifest.world_id.empty())
        return Result<WorldCellContent>::failure({ErrorCode::invalid_argument, "World cell content requires a loaded manifest"});
    const auto declared = std::find_if(manifest.cells.begin(), manifest.cells.end(),
        [&cell](const WorldCell& item) { return item.id == cell.id && item.content_path == cell.content_path; });
    if (declared == manifest.cells.end())
        return Result<WorldCellContent>::failure({ErrorCode::invalid_argument, "World cell is not declared by this manifest"});
    if (max_bytes == 0)
        return Result<WorldCellContent>::failure({ErrorCode::invalid_argument, "World cell content size limit must be positive"});
    auto relative = platform::safe_relative_path(cell.content_path);
    if (!relative) return Result<WorldCellContent>::failure(relative.error());
    std::error_code ec;
    const auto root = std::filesystem::canonical(manifest.source_path.parent_path(), ec);
    if (ec) return Result<WorldCellContent>::failure({ErrorCode::io_error, "Cannot resolve world manifest directory: " + ec.message()});
    const auto source = std::filesystem::canonical(root / relative.value(), ec);
    if (ec) return Result<WorldCellContent>::failure({ErrorCode::io_error, "Cannot resolve world cell content '" + cell.content_path.generic_string() + "': " + ec.message()});
    if (!path_is_within(root, source))
        return Result<WorldCellContent>::failure({ErrorCode::path_escape, "World cell content resolves outside the manifest directory"});
    const auto size = std::filesystem::file_size(source, ec);
    if (ec) return Result<WorldCellContent>::failure({ErrorCode::io_error, "Cannot inspect world cell content size: " + ec.message()});
    if (size > max_bytes || size > static_cast<std::uintmax_t>(std::numeric_limits<std::streamsize>::max()))
        return Result<WorldCellContent>::failure({ErrorCode::invalid_argument, "World cell content exceeds the configured size limit"});
    std::ifstream input(source, std::ios::binary);
    if (!input) return Result<WorldCellContent>::failure({ErrorCode::io_error, "Cannot open world cell content: " + source.string()});
    std::string contents(static_cast<std::size_t>(size), '\0');
    if (size != 0) input.read(contents.data(), static_cast<std::streamsize>(size));
    if (!input || input.peek() != std::char_traits<char>::eof())
        return Result<WorldCellContent>::failure({ErrorCode::io_error, "Failed to read complete world cell content"});
    auto scene = deserialize_scene(contents);
    if (!scene) return Result<WorldCellContent>::failure({scene.error().code, "Invalid content for world cell '" + cell.id + "': " + scene.error().message});
    return Result<WorldCellContent>::success(WorldCellContent{cell.id, source, std::move(scene.value())});
}
} // namespace ecos::studio
