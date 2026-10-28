#include "ecos_studio/project.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace fs = std::filesystem;
namespace {
int failures = 0;
void check(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
std::string read(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}
void write(const fs::path& path, const std::string& contents) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << contents;
}
}
int main() {
    const auto root = fs::temp_directory_path() / "ecos-studio-mod-manifest-tests";
    std::error_code ec;
    fs::remove_all(root, ec);
    std::string error;
    check(ecos::studio::create_mod(root, "com.example.demo", error), "create mod scaffold");
    const auto manifest = read(root / "mod.ecos");
    check(manifest.find("dependencies=none") != std::string::npos, "scaffold declares no dependencies explicitly");
    check(ecos::studio::validate_mod(root).ok, "generated mod validates");

    auto dependencies = manifest;
    auto marker = dependencies.find("dependencies=none");
    if (marker != std::string::npos) dependencies.replace(marker, std::string("dependencies=none").size(), "dependencies=com.example.base,com.example.shared");
    write(root / "mod.ecos", dependencies);
    check(ecos::studio::validate_mod(root).ok, "multiple valid dependency IDs validate");

    auto self_dependency = manifest;
    marker = self_dependency.find("dependencies=none");
    if (marker != std::string::npos) self_dependency.replace(marker, std::string("dependencies=none").size(), "dependencies=com.example.demo");
    write(root / "mod.ecos", self_dependency);
    check(!ecos::studio::validate_mod(root).ok, "self dependency rejected");

    auto duplicate = manifest;
    marker = duplicate.find("dependencies=none");
    if (marker != std::string::npos) duplicate.replace(marker, std::string("dependencies=none").size(), "dependencies=com.example.base,com.example.base");
    write(root / "mod.ecos", duplicate);
    check(!ecos::studio::validate_mod(root).ok, "duplicate dependency rejected");

    auto malformed = manifest;
    marker = malformed.find("dependencies=none");
    if (marker != std::string::npos) malformed.replace(marker, std::string("dependencies=none").size(), "dependencies=../unsafe");
    write(root / "mod.ecos", malformed);
    check(!ecos::studio::validate_mod(root).ok, "malformed dependency ID rejected");
    fs::remove_all(root, ec);
    if (failures) { std::cerr << failures << " test(s) failed.\n"; return 1; }
    std::cout << "ECOS Studio mod manifest tests passed.\n";
    return 0;
}
