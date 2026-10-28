#include "ecos_studio/project.hpp"
#include "ecos_studio/scene.hpp"

#include <cctype>
#include <fstream>
#include <map>
#include <sstream>
#include <system_error>

namespace ecos::studio {
namespace fs = std::filesystem;
namespace {

using Manifest = std::map<std::string, std::string>;

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

bool safe_relative_path(const std::string& value) {
    if (value.empty()) return false;
    const fs::path path(value);
    if (path.is_absolute() || path.has_root_name()) return false;
    for (const auto& part : path) if (part == "..") return false;
    return true;
}

bool parse_manifest(const fs::path& path, Manifest& manifest, std::string& error) {
    std::ifstream input(path);
    if (!input) {
        error = "Cannot open manifest: " + path.string();
        return false;
    }
    std::string line;
    std::size_t line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        line = trim(line);
        if (line.empty() || line.front() == '#') continue;
        const auto separator = line.find('=');
        if (separator == std::string::npos) {
            error = "Expected key=value at " + path.string() + ":" + std::to_string(line_number);
            return false;
        }
        const auto key = trim(line.substr(0, separator));
        const auto value = trim(line.substr(separator + 1));
        if (key.empty() || value.empty()) {
            error = "Empty key or value at " + path.string() + ":" + std::to_string(line_number);
            return false;
        }
        if (!manifest.emplace(key, value).second) {
            error = "Duplicate key '" + key + "' in " + path.string();
            return false;
        }
    }
    return true;
}

bool has_value(const Manifest& manifest, const std::string& key, const std::string& expected) {
    const auto it = manifest.find(key);
    return it != manifest.end() && it->second == expected;
}

bool require_value(const Manifest& manifest, const std::string& key, ValidationResult& result) {
    const auto it = manifest.find(key);
    if (it == manifest.end() || it->second.empty()) {
        result.ok = false;
        result.errors.push_back("Missing required field: " + key);
        return false;
    }
    return true;
}

bool valid_mod_id(const std::string& id) {
    if (id.empty() || !std::isalnum(static_cast<unsigned char>(id.front()))) return false;
    for (const unsigned char c : id) {
        if (!(std::islower(c) || std::isdigit(c) || c == '.' || c == '-' || c == '_')) return false;
    }
    return true;
}

bool write_file(const fs::path& path, const std::string& contents, std::string& error) {
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        error = "Cannot write file: " + path.string();
        return false;
    }
    output << contents;
    if (!output) {
        error = "Failed while writing file: " + path.string();
        return false;
    }
    return true;
}

bool ensure_directory(const fs::path& path, std::string& error) {
    std::error_code ec;
    fs::create_directories(path, ec);
    if (ec) {
        error = "Cannot create directory '" + path.string() + "': " + ec.message();
        return false;
    }
    return true;
}

} // namespace

bool create_project(const fs::path& root, const std::string& name, std::string& error) {
    if (trim(name).empty() || name.find_first_of("\r\n=") != std::string::npos) {
        error = "Project name must be non-empty and cannot contain newlines or '='.";
        return false;
    }
    std::error_code ec;
    if (fs::exists(root / "project.ecos", ec)) {
        error = "A project.ecos already exists at the destination; refusing to overwrite it.";
        return false;
    }
    if (!ensure_directory(root, error)) return false;
    for (const auto* directory : {"scenes", "assets", "scripts", "mods"}) {
        if (!ensure_directory(root / directory, error)) return false;
    }
    const std::string manifest =
        "# ECOS Studio project manifest; format_version is intentionally explicit.\n"
        "format_version=1\nname=" + name +
        "\nproject_type=game\nworld_mode=streamed\nentry_scene=scenes/main.ecscene\n";
    if (!write_file(root / "project.ecos", manifest, error)) return false;
    const std::string scene =
        "# ECOS Studio scene document v1.\n"
        "format_version=1\nscene_id=main\n"
        "node=1|-|WorldRoot|0,0,0|0,0,0|1,1,1\n";
    if (!write_file(root / "scenes/main.ecscene", scene, error)) return false;
    const std::string ignore =
        "# Generated/imported content should not be committed by default.\n"
        "build/\n.ecos-cache/\n";
    return write_file(root / ".gitignore", ignore, error);
}

ValidationResult validate_project(const fs::path& root) {
    ValidationResult result;
    Manifest manifest;
    std::string error;
    if (!parse_manifest(root / "project.ecos", manifest, error)) {
        result.ok = false;
        result.errors.push_back(error);
        return result;
    }
    for (const auto* key : {"format_version", "name", "project_type", "world_mode", "entry_scene"})
        require_value(manifest, key, result);
    if (!has_value(manifest, "format_version", "1")) {
        result.ok = false;
        result.errors.push_back("Unsupported format_version; this bootstrap supports version 1.");
    }
    if (!has_value(manifest, "project_type", "game")) {
        result.ok = false;
        result.errors.push_back("project_type must be 'game'.");
    }
    const auto world_mode = manifest.find("world_mode");
    if (world_mode != manifest.end() && world_mode->second != "streamed" && world_mode->second != "single_scene") {
        result.ok = false;
        result.errors.push_back("world_mode must be 'streamed' or 'single_scene'.");
    }
    const auto entry = manifest.find("entry_scene");
    if (entry != manifest.end()) {
        if (!safe_relative_path(entry->second)) {
            result.ok = false;
            result.errors.push_back("entry_scene must be a safe relative path (no absolute paths or '..').");
        } else if (!fs::is_regular_file(root / entry->second)) {
            result.ok = false;
            result.errors.push_back("entry_scene does not exist: " + entry->second);
        } else if (fs::path(entry->second).extension() == ".ecscene") {
            std::ifstream scene_input(root / entry->second, std::ios::binary);
            std::ostringstream scene_buffer;
            scene_buffer << scene_input.rdbuf();
            auto scene = deserialize_scene(scene_buffer.str());
            if (!scene) {
                result.ok = false;
                result.errors.push_back("Invalid entry scene: " + scene.error().message);
            }
        }
    }
    for (const auto& [key, value] : manifest) {
        (void)value;
        if (key != "format_version" && key != "name" && key != "project_type" &&
            key != "world_mode" && key != "entry_scene")
            result.warnings.push_back("Unknown field will be ignored by this bootstrap: " + key);
    }
    return result;
}

bool create_mod(const fs::path& root, const std::string& id, std::string& error) {
    if (!valid_mod_id(id)) {
        error = "Mod ID must start with a letter or digit and contain only lowercase letters, digits, '.', '-' or '_'.";
        return false;
    }
    std::error_code ec;
    if (fs::exists(root / "mod.ecos", ec)) {
        error = "A mod.ecos already exists at the destination; refusing to overwrite it.";
        return false;
    }
    if (!ensure_directory(root, error)) return false;
    for (const auto* directory : {"assets", "data"}) {
        if (!ensure_directory(root / directory, error)) return false;
    }
    const std::string manifest =
        "# Data-only mod manifest; native code is not executed by this bootstrap.\n"
        "format_version=1\nid=" + id + "\nname=" + id +
        "\nversion=0.1.0\ntarget_api=1\nentry=README.md\n";
    if (!write_file(root / "mod.ecos", manifest, error)) return false;
    const std::string readme =
        "# Mod package\n\n"
        "This is a data-only ECOS Studio mod scaffold. Add supported assets and data under assets/ and data/. "
        "The bootstrap does not execute native code.\n";
    return write_file(root / "README.md", readme, error);
}

ValidationResult validate_mod(const fs::path& root) {
    ValidationResult result;
    Manifest manifest;
    std::string error;
    if (!parse_manifest(root / "mod.ecos", manifest, error)) {
        result.ok = false;
        result.errors.push_back(error);
        return result;
    }
    for (const auto* key : {"format_version", "id", "name", "version", "target_api", "entry"})
        require_value(manifest, key, result);
    const auto id = manifest.find("id");
    if (id != manifest.end() && !valid_mod_id(id->second)) {
        result.ok = false;
        result.errors.push_back("Invalid mod id: use lowercase letters, digits, '.', '-' or '_', starting with a letter or digit.");
    }
    if (!has_value(manifest, "format_version", "1")) {
        result.ok = false;
        result.errors.push_back("Unsupported mod format_version; this bootstrap supports version 1.");
    }
    if (!has_value(manifest, "target_api", "1")) {
        result.ok = false;
        result.errors.push_back("Unsupported target_api; this bootstrap supports API version 1.");
    }
    const auto entry = manifest.find("entry");
    if (entry != manifest.end()) {
        if (!safe_relative_path(entry->second)) {
            result.ok = false;
            result.errors.push_back("entry must be a safe relative path (no absolute paths or '..').");
        } else if (!fs::is_regular_file(root / entry->second)) {
            result.ok = false;
            result.errors.push_back("Mod entry file does not exist: " + entry->second);
        }
    }
    return result;
}

} // namespace ecos::studio
