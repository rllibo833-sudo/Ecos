#include "ecos_studio/package_manager.hpp"
#include "ecos_studio/project.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace fs = std::filesystem;
namespace {
int failures = 0;
void check(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
std::string read(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}
void write(const fs::path& path, const std::string& contents) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << contents;
}
void set_dependencies(const fs::path& root, const std::string& dependencies) {
    auto manifest = read(root / "mod.ecos");
    const auto marker = manifest.find("dependencies=none");
    if (marker != std::string::npos)
        manifest.replace(marker, std::string("dependencies=none").size(), "dependencies=" + dependencies);
    write(root / "mod.ecos", manifest);
}
bool contains(const std::vector<std::string>& values, const std::string& part) {
    for (const auto& value : values) if (value.find(part) != std::string::npos) return true;
    return false;
}
}

int main() {
    const auto root = fs::temp_directory_path() / "ecos-studio-package-manager-tests";
    std::error_code ec;
    fs::remove_all(root, ec);
    fs::create_directories(root, ec);
    check(!ec, "create temporary mods root");

    std::string error;
    const auto base = root / "base";
    const auto shared = root / "shared";
    const auto gameplay = root / "gameplay";
    check(ecos::studio::create_mod(base, "com.example.base", error), "create base package");
    check(ecos::studio::create_mod(shared, "com.example.shared", error), "create shared package");
    check(ecos::studio::create_mod(gameplay, "com.example.gameplay", error), "create gameplay package");
    set_dependencies(gameplay, "com.example.base,com.example.shared");

    auto resolved = ecos::studio::resolve_mod_dependencies(root);
    check(resolved.ok, "resolve valid package graph");
    check(resolved.load_order.size() == 3, "include every installed package");
    const auto base_pos = std::find(resolved.load_order.begin(), resolved.load_order.end(), "com.example.base");
    const auto shared_pos = std::find(resolved.load_order.begin(), resolved.load_order.end(), "com.example.shared");
    const auto gameplay_pos = std::find(resolved.load_order.begin(), resolved.load_order.end(), "com.example.gameplay");
    check(base_pos != resolved.load_order.end() && gameplay_pos != resolved.load_order.end() && base_pos < gameplay_pos,
          "dependency loads before dependent");
    check(shared_pos != resolved.load_order.end() && gameplay_pos != resolved.load_order.end() && shared_pos < gameplay_pos,
          "all dependencies load before dependent");
    const auto first_order = resolved.load_order;
    resolved = ecos::studio::resolve_mod_dependencies(root);
    check(resolved.load_order == first_order, "load order is deterministic");

    set_dependencies(gameplay, "com.example.not-installed");
    resolved = ecos::studio::resolve_mod_dependencies(root);
    check(!resolved.ok && contains(resolved.errors, "missing dependency"), "missing dependency rejected");

    set_dependencies(gameplay, "com.example.base");
    set_dependencies(base, "com.example.gameplay");
    resolved = ecos::studio::resolve_mod_dependencies(root);
    check(!resolved.ok && contains(resolved.errors, "Dependency cycle"), "dependency cycle rejected");

    resolved = ecos::studio::resolve_mod_dependencies(root / "does-not-exist");
    check(!resolved.ok, "missing mods root rejected");

    fs::remove_all(root, ec);
    if (failures) { std::cerr << failures << " test(s) failed.\n"; return 1; }
    std::cout << "ECOS Studio package manager tests passed.\n";
    return 0;
}
