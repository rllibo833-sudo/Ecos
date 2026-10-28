#include "ecos_studio/package_manager.hpp"

#include "ecos_studio/project.hpp"

#include <algorithm>
#include <fstream>
#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <system_error>

namespace ecos::studio {
namespace fs = std::filesystem;
namespace {

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

bool read_manifest_field(const fs::path& manifest_path, const std::string& wanted,
                         std::string& value, std::string& error) {
    std::ifstream input(manifest_path);
    if (!input) {
        error = "Cannot open mod manifest: " + manifest_path.string();
        return false;
    }
    std::string line;
    while (std::getline(input, line)) {
        line = trim(std::move(line));
        if (line.empty() || line.front() == '#') continue;
        const auto equals = line.find('=');
        if (equals == std::string::npos) continue;
        if (trim(line.substr(0, equals)) == wanted) {
            value = trim(line.substr(equals + 1));
            return true;
        }
    }
    error = "Missing '" + wanted + "' field in " + manifest_path.string();
    return false;
}

std::vector<std::string> split_dependencies(const std::string& value) {
    if (value.empty() || value == "none") return {};
    std::vector<std::string> result;
    std::istringstream input(value);
    std::string item;
    while (std::getline(input, item, ',')) result.push_back(trim(std::move(item)));
    return result;
}

struct Package {
    fs::path path;
    std::vector<std::string> dependencies;
};

} // namespace

ModResolution resolve_mod_dependencies(const fs::path& mods_root) {
    ModResolution result;
    std::error_code ec;
    const auto root_status = fs::symlink_status(mods_root, ec);
    if (ec || !fs::is_directory(root_status) || fs::is_symlink(root_status)) {
        result.ok = false;
        result.errors.push_back("Mods root must be an existing, non-symlink directory: " + mods_root.string());
        return result;
    }

    std::map<std::string, Package> packages;
    fs::directory_iterator iterator(mods_root, ec), end;
    if (ec) {
        result.ok = false;
        result.errors.push_back("Cannot scan mods root '" + mods_root.string() + "': " + ec.message());
        return result;
    }
    for (; iterator != end; iterator.increment(ec)) {
        if (ec) {
            result.ok = false;
            result.errors.push_back("Failed while scanning mods root: " + ec.message());
            break;
        }
        const auto status = iterator->symlink_status(ec);
        if (ec) {
            result.ok = false;
            result.errors.push_back("Cannot inspect package entry '" + iterator->path().string() + "': " + ec.message());
            ec.clear();
            continue;
        }
        if (fs::is_symlink(status)) {
            result.warnings.push_back("Ignoring symlink in mods root: " + iterator->path().filename().string());
            continue;
        }
        if (!fs::is_directory(status)) continue;

        const auto manifest_path = iterator->path() / "mod.ecos";
        if (!fs::is_regular_file(manifest_path, ec)) {
            ec.clear();
            continue; // Non-package directories are allowed in the workspace.
        }
        const auto validation = validate_mod(iterator->path());
        if (!validation.ok) {
            result.ok = false;
            for (const auto& message : validation.errors)
                result.errors.push_back(iterator->path().filename().string() + ": " + message);
            continue;
        }

        std::string id, dependency_text, field_error;
        if (!read_manifest_field(manifest_path, "id", id, field_error) ||
            !read_manifest_field(manifest_path, "dependencies", dependency_text, field_error)) {
            result.ok = false;
            result.errors.push_back(field_error);
            continue;
        }
        Package package{iterator->path(), split_dependencies(dependency_text)};
        if (!packages.emplace(id, std::move(package)).second) {
            result.ok = false;
            result.errors.push_back("Duplicate installed mod ID: " + id);
        }
    }

    for (const auto& [id, package] : packages) {
        for (const auto& dependency : package.dependencies) {
            if (packages.find(dependency) == packages.end()) {
                result.ok = false;
                result.errors.push_back("Mod '" + id + "' requires missing dependency '" + dependency + "'.");
            }
        }
    }

    // DFS over sorted package IDs and manifest dependency order gives a stable load order.
    enum class Visit { unseen, visiting, visited };
    std::map<std::string, Visit> visits;
    std::vector<std::string> stack;
    std::set<std::string> reported_cycles;
    std::function<void(const std::string&)> visit = [&](const std::string& id) {
        const auto state = visits[id];
        if (state == Visit::visited) return;
        if (state == Visit::visiting) {
            const auto start = std::find(stack.begin(), stack.end(), id);
            std::ostringstream cycle;
            cycle << "Dependency cycle: ";
            for (auto it = start; it != stack.end(); ++it) cycle << *it << " -> ";
            cycle << id;
            const auto message = cycle.str();
            if (reported_cycles.insert(message).second) {
                result.ok = false;
                result.errors.push_back(message);
            }
            return;
        }
        visits[id] = Visit::visiting;
        stack.push_back(id);
        const auto found = packages.find(id);
        if (found != packages.end()) {
            for (const auto& dependency : found->second.dependencies)
                if (packages.find(dependency) != packages.end()) visit(dependency);
        }
        stack.pop_back();
        visits[id] = Visit::visited;
        result.load_order.push_back(id);
    };

    for (const auto& [id, package] : packages) {
        (void)package;
        visit(id);
    }
    if (!result.ok) result.load_order.clear();
    return result;
}

} // namespace ecos::studio
